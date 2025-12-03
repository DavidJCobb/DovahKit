#include "./IdleAnimationFormsModel.h"
#include <QColor>
#include <QMessageBox>
#pragma region Drag and drop
   #include <QByteArray>
   #include <QDataStream>
   #include <QMimeData>
#pragma endregion
#include "dovah/datastores/idles/action_node.h"
#include "dovah/datastores/idles/graph_node.h"
#include "dovah/datastores/idles/idle_node.h"
#include "dovah/datastores/idles/loose_idle_list_node.h"
#include "dovah/forms/IdleAnimation.h"
#include "dovah/form_stub.h"
#include "dovahscript/dovahscript_host.h"
#include "editor/core.h"
#include "editor/form_stub_meta_type.h"
#include "editor/helpers/make_editor_id_for_duplicate.h"
#include "editor/subsystems/message_log/core.h"
#include "dovah/files/tes_file_reading/file_loader.h"
#include "ui/types/game_file_path.h"

// for pushing warnings to the log window:
#include "dovah/datastores/idles/warnings/action_root_is_forced_loose.h"
#include "dovah/datastores/idles/warnings/child_idle_is_an_action_root_candidate.h"
#include "dovah/datastores/idles/warnings/child_of_idle_is_forced_loose.h"
#include "dovah/datastores/idles/warnings/cyclical_parent_relationships.h"
#include "dovah/datastores/idles/warnings/cyclical_sibling_relationships.h"
#include "dovah/datastores/idles/warnings/idle_has_candidacies_for_multiple_actions.h"
#include "dovah/datastores/idles/warnings/idle_has_multiple_next_siblings.h"
#include "dovah/datastores/idles/warnings/inconsistent_parentage_on_idle.h"
#include "dovah/datastores/idles/warnings/loose_idle_is_not_flagged.h"
#include "dovah/datastores/idles/warnings/no_loose_idle_list_for_idle.h"
#include "dovah/datastores/idles/warnings/orphaned_idle.h"
#include "dovah/datastores/idles/warnings/previous_sibling_is_not_as_expected.h"
#include "dovah/datastores/idles/warnings/siblings_have_mismatched_parents.h"
#include "dovah/datastores/idles/warnings/sibling_is_an_ancestor.h"
#include "editor/helpers/form_identifiers_to_string.h"

namespace {
   constexpr const char* const mime_type = "application/dovah-kit.idle-animation-forms-model.node";
}
namespace datastore_warnings {
   using namespace dovah::datastores::impl::idles::warnings;
}

IdleAnimationFormsModel::IdleAnimationFormsModel(QObject* parent) : QAbstractItemModel(parent) {
   #pragma region Set up datastore handlers
      this->_datastore.handlers.delete_idle = [this](dovah::form_stub& stub) {
         DovahKitCore::get().delete_form(
            stub,
            [](const dovah::form_deletion_request&) {
               return true;
            },
            [this](const dovah::exceptions::form_deletion_failed& ex) {
               this->_handler_state.any_deletions_failed = true;
            },
            [](const dovah::form_deletion_request& request) {}
         );
      };
   #pragma endregion
   #pragma region Set up datastore callbacks
      #pragma region Global
         {
            auto& cb_set = this->_datastore.callbacks.reset;
            cb_set.before = [this]() { this->beginResetModel(); };
            cb_set.after = [this]() { this->endResetModel(); };
         }
         {
            auto& cb_set = this->_datastore.callbacks.form_data_modified;
            cb_set.before = [this](dovah::form_stub& stub) {
               DovahKitCore::get().formModificationImminent(&stub);
            };
            cb_set.after = [this](dovah::form_stub& stub) {
               DovahKitCore::get().formModified(&stub);
            };
         }
      #pragma endregion
      #pragma region Graphs
         {
            auto& cb_set = this->_datastore.callbacks.graph_inserted;
            cb_set.before = [this](const graph_node&, size_t index) {
               this->beginInsertRows(_qmi_for_model_root(), index, index);
            };
            cb_set.after = [this](const graph_node&) {
               this->endInsertRows();
            };
         }
      #pragma endregion
      #pragma region Actions
         {
            auto& cb_set = this->_datastore.callbacks.action_deleted;
            cb_set.before = [this](const action_node& node) {
               if (node.graph) {
                  auto qmi = _qmi_for_node(*node.graph);
                  auto i   = node.graph->index_of_action(node);
                  this->beginRemoveRows(qmi, i, i);
                  this->_callback_state.emitted_last_deletion = true;
               }
            };
            cb_set.after = [this]() {
               if (this->_callback_state.emitted_last_deletion)
                  this->endRemoveRows();
               this->_callback_state.emitted_last_deletion = false;
            };
         }
         {
            auto& cb_set = this->_datastore.callbacks.action_inserted;
            cb_set.before = [this](const graph_node& graph, const action_node& action, size_t index) {
               this->_callback_state.last_node_placement_was_an_insertion = true;
               this->beginInsertRows(_qmi_for_node(graph), index, index);
            };
            cb_set.after = [this](const action_node& node) {
               this->_recache_action(node);
               if (this->_callback_state.last_node_placement_was_an_insertion)
                  this->endInsertRows();
               else
                  this->endMoveRows();
            };
         }
      #pragma endregion
      #pragma region Idles
         {
            auto& cb_set = this->_datastore.callbacks.idle_deleted;
            cb_set.before = [this](const idle_node& node) {
               if (node.canonical_parent) {
                  auto qmi = _qmi_for_node(*node.canonical_parent);
                  size_t i = 0;
                  if (auto* casted = dynamic_cast<idle_node*>(node.canonical_parent)) {
                     i = casted->index_of_child(node);
                  } else if (auto* casted = dynamic_cast<loose_idle_list_node*>(node.canonical_parent)) {
                     i = casted->index_of_child(node);
                  }
                  this->beginRemoveRows(qmi, i, i);
                  this->_callback_state.emitted_last_deletion = true;
               }
            };
            cb_set.after = [this](const uint32_t form_id) {
               if (this->_callback_state.emitted_last_deletion)
                  this->endRemoveRows();
               this->_callback_state.emitted_last_deletion = false;
            };
         }
         {
            auto& cb_set = this->_datastore.callbacks.idle_moved;
            cb_set.before = [this](const idle_node& idle, const datastore_node& parent_after, size_t index) {
               if (idle.canonical_parent) {
                  auto&  parent_prior = *idle.canonical_parent;
                  size_t from = 0;
                  if (auto* casted = dynamic_cast<idle_node*>(idle.canonical_parent)) {
                     from = casted->index_of_child(idle);
                  } else if (auto* casted = dynamic_cast<loose_idle_list_node*>(idle.canonical_parent)) {
                     from = casted->index_of_child(idle);
                  }
                  auto to        = index;
                  auto qmi_prior = _qmi_for_node(parent_prior);
                  auto qmi_after = _qmi_for_node(parent_after);
                  if (to >= from)
                     ++to;
                  this->_callback_state.last_node_placement_was_an_insertion = false;
                  this->beginMoveRows(qmi_prior, from, from, qmi_after, to);
               } else {
                  this->_callback_state.last_node_placement_was_an_insertion = true;
                  this->beginInsertRows(_qmi_for_node(parent_after), index, index);
               }
            };
            cb_set.after = [this](const idle_node& node) {
               this->_recache_idle(node);
               if (this->_callback_state.last_node_placement_was_an_insertion)
                  this->endInsertRows();
               else
                  this->endMoveRows();
            };
         }
         this->_datastore.callbacks.idle_becoming_multiply_present_in = [this](idle_node& idle, action_node& action) {
            this->beginInsertRows(_qmi_for_node(action), 0, 0);
            this->endInsertRows();
         };
         this->_datastore.callbacks.idle_no_longer_multiply_present_in = [this](idle_node& idle, action_node& action) {
            this->beginRemoveRows(_qmi_for_node(action), 0, 0);
            this->endRemoveRows();
         };
      #pragma endregion
   #pragma endregion

   auto& scripthost = DovahscriptHost::get();
   QObject::connect(&scripthost, &DovahscriptHost::scriptEnded, this, [this]() {
      //
      // Dovahscript APIs could allow a script to modify an idle's parent and/or previous sibling 
      // such that its position in the hierarchy is invalid. Error-checking those after the tree 
      // is already built is bloody difficult, so the datastore currently doesn't implement that. 
      // The only recourse we have, for any situation where an idle's graph, parent, or previous 
      // sibling might be changed out from udner the datastore, is to rebuild the datastore from 
      // scratch.
      //
      this->_rebuild_datastore();
   });

   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent,  this, &IdleAnimationFormsModel::_on_game_data_abandon);
   QObject::connect(&editor, &DovahKitCore::dataAcquireComplete,  this, &IdleAnimationFormsModel::_on_game_data_acquired);
   QObject::connect(&editor, &DovahKitCore::formCreated,          this, [this](dovah::form_stub* stub) { this->_on_form_created(*stub); });
   QObject::connect(&editor, &DovahKitCore::formModified,         this, [this](dovah::form_stub* stub) { this->_on_form_modified(*stub); });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub, bool flag) { this->_on_form_deletion_imminent(*stub, flag); });
   QObject::connect(&editor, &DovahKitCore::formDeletionComplete, this, [this](dovah::bare_form_id_t id, bool flag) { this->_on_form_deleted(id, flag); });
   if (editor.has_data()) {
      this->_rebuild_datastore();
   }
}

void IdleAnimationFormsModel::validateForDebug() {
   #if _DEBUG
      #pragma region Verify that all reachable nodes have cached data for the UI
         //
         // Cached data is used at display time so that we're not converting a bunch 
         // of std::strings to QStrings on every single frame.
         //
         for (auto& pair : this->_datastore.idles_by_stub) {
            auto it = this->_cache.find(pair.second);
            if (it == this->_cache.end())
               __debugbreak(); // idle has no UI-side cache!
         }
         for (const graph_node* graph : this->_datastore.graphs) {
            auto it = this->_cache.find((graph_node*)graph); // unordered_map chokes on lookups by const-pointer keys if the base key type isn't const
            if (it == this->_cache.end())
               __debugbreak(); // graph has no UI-side cache!
            for (const action_node* action : graph->actions) {
               auto it = this->_cache.find((action_node*)action); // unordered_map chokes on lookups by const-pointer keys if the base key type isn't const
               if (it == this->_cache.end())
                  __debugbreak(); // action has no UI-side cache!
            }
         }
      #pragma endregion
      this->_datastore.debug_verify_integrity();
      #pragma region Diff the datastore
      {
         auto* flo = DovahKitCore::get().get_file_load_order();
         if (flo) {
            datastore_type diff;
            diff.build(*flo);
            this->_datastore.debug_do_semantic_compare(diff);
         }
      }
      #pragma endregion
   #endif
}

#pragma region Node/QMI utils and node lookups
   QModelIndex IdleAnimationFormsModel::_qmi_for_model_root() const {
      return {};
   }
   QModelIndex IdleAnimationFormsModel::_qmi_for_node(const datastore_node& n, int column) const {
      if (column < 0)
         return {};
      const auto& graphs = this->_datastore.graphs;
      const auto  g_size = graphs.size();

      if (&n == this->_datastore.loose) {
         if (this->_datastore.loose->child_idles.empty())
            return {};
         return this->createIndex(this->_datastore.graphs.size(), column, nullptr);
      }

      const void* parent_void = nullptr;
      int         row         = -1;
      if (auto* casted = dynamic_cast<const action_node*>(&n)) {
         const graph_node* parent = casted->graph;
         parent_void = parent;
         if (parent)
            row = parent->index_of_action(*casted);
      } else if (auto* casted = dynamic_cast<const idle_node*>(&n)) {
         parent_void = casted->canonical_parent;
         if (auto* casted_parent = dynamic_cast<action_node*>(casted->canonical_parent)) {
            row = 0;
         } else if (auto* casted_parent = dynamic_cast<loose_idle_list_node*>(casted->canonical_parent)) {
            row = casted_parent->index_of_child(*casted);
         } else if (auto* casted_parent = dynamic_cast<idle_node*>(casted->canonical_parent)) {
            row = casted_parent->index_of_child(*casted);
         }
      } else if (auto* casted = dynamic_cast<const graph_node*>(&n)) {
         for (size_t i = 0; i < g_size; ++i) {
            if (graphs[i] == casted) {
               row = i;
               break;
            }
         }
      } else if (auto* casted = dynamic_cast<const loose_idle_list_node*>(&n)) {
         const graph_node* parent = casted->graph;
         parent_void = parent;
         if (parent)
            row = parent->actions.size();
      }
      if (row == -1)
         return {};
      return this->createIndex(row, column, (quintptr)parent_void);
   }
   QModelIndex IdleAnimationFormsModel::_qmi_for_child_node(int row, int column, const datastore_node& parent) const {
      return this->createIndex(row, column, (quintptr)&parent);
   }
   bool IdleAnimationFormsModel::_qmi_is_child_of(const QModelIndex& qmi, const datastore_node& node) const {
      return qmi.internalPointer() == &node;
   }

   const IdleAnimationFormsModel::datastore_node* IdleAnimationFormsModel::_child_node_by_row(const datastore_node* parent, size_t row) const {
      if (!parent) {
         const auto&  list = this->_datastore.graphs;
         const size_t size = list.size();
         if (row < size)
            return list[row];
         if (row == size) {
            auto* loose = this->_datastore.loose;
            if (!loose->child_idles.empty())
               return loose;
         }
         return nullptr;
      }
      if (auto* casted = dynamic_cast<const graph_node*>(parent)) {
         const auto&  list = casted->actions;
         const size_t size = list.size();
         if (row < size)
            return list[row];
         if (row == size)
            return casted->loose;
         return nullptr;
      }
      if (auto* casted = dynamic_cast<const action_node*>(parent)) {
         if (row == 0)
            return casted->winning_root;
         return nullptr;
      }
      if (auto* casted = dynamic_cast<const idle_node*>(parent)) {
         const auto&  list = casted->child_idles;
         const size_t size = list.size();
         if (row < size)
            return list[row];
         return nullptr;
      }
      if (auto* casted = dynamic_cast<const loose_idle_list_node*>(parent)) {
         const auto&  list = casted->child_idles;
         const size_t size = list.size();
         if (row < size)
            return list[row];
         return nullptr;
      }
      return nullptr;
   }

   const IdleAnimationFormsModel::datastore_node* IdleAnimationFormsModel::_node_for_qmi(const QModelIndex& qmi) const {
      if (!qmi.isValid())
         return nullptr;
      auto* parent = (datastore_node*)qmi.internalPointer();
      return _child_node_by_row(parent, qmi.row());
   }
   IdleAnimationFormsModel::datastore_node* IdleAnimationFormsModel::_node_for_qmi(const QModelIndex& qmi) {
      return const_cast<datastore_node*>(std::as_const(*this)._node_for_qmi(qmi));
   }

   const IdleAnimationFormsModel::datastore_node* IdleAnimationFormsModel::_child_node_for_qmi(const QModelIndex& parent_qmi, int row) const {
      if (parent_qmi == _qmi_for_model_root()) {
         return _child_node_by_row(nullptr, row);
      }
      const auto* parent_node = _node_for_qmi(parent_qmi);
      if (!parent_node)
         return nullptr;
      return _child_node_by_row(parent_node, row);
   }
   IdleAnimationFormsModel::datastore_node* IdleAnimationFormsModel::_child_node_for_qmi(const QModelIndex& parent_qmi, int row) {
      return const_cast<datastore_node*>(std::as_const(*this)._child_node_for_qmi(parent_qmi, row));
   }

   const IdleAnimationFormsModel::graph_node* IdleAnimationFormsModel::_node_for_graph_path(QString path) const {
      for (auto* graph : this->_datastore.graphs) {
         {
            auto it = this->_cache.find(graph);
            if (it != this->_cache.end()) {
               if (it->second.display_string.compare(path, Qt::CaseInsensitive) == 0)
                  return graph;
            }
         }
         QString graph_path = QString::fromStdString(graph->path);
         if (graph_path.compare(path, Qt::CaseInsensitive) == 0)
            return graph;
      }
      return nullptr;
   }
   IdleAnimationFormsModel::graph_node* IdleAnimationFormsModel::_node_for_graph_path(QString path) {
      return const_cast<graph_node*>(std::as_const(*this)._node_for_graph_path(path));
   }
   
   IdleAnimationFormsModel::action_node* IdleAnimationFormsModel::_node_for_action(graph_node& graph, const dovah::form_stub& stub) {
      for (action_node* node : graph.actions)
         if (&node->stub == &stub)
            return node;
      return nullptr;
   }
#pragma endregion

#pragma region Constraints
   bool IdleAnimationFormsModel::_can_create_new_idle_in(const datastore_node& parent) const {
      if (auto* as_action = dynamic_cast<const action_node*>(&parent)) {
         return as_action->winning_root == nullptr;
      }
      if (dynamic_cast<const loose_idle_list_node*>(&parent)) {
         return true;
      }
      if (auto* as_idle = dynamic_cast<const idle_node*>(&parent)) {
         if (!as_idle->child_idles.empty())
            return true;
         //
         // Don't allow adding children to loose idles.
         //
         if (dynamic_cast<const loose_idle_list_node*>(as_idle->canonical_parent))
            return false;
         return true;
      }
      return true;
   }
   bool IdleAnimationFormsModel::_can_ever_duplicate(const idle_node& n) const {
      if (!n.canonical_parent)
         return false;
      if (dynamic_cast<const action_node*>((datastore_node*)n.canonical_parent))
         return false;
      return true;
   }
#pragma endregion

#pragma region Form events
   void IdleAnimationFormsModel::_on_game_data_acquired() {
      this->_rebuild_datastore();
   }
   void IdleAnimationFormsModel::_on_game_data_abandon() {
      this->beginResetModel();
      this->_cache.clear();
      this->_drag_and_drop.clear();
      this->_datastore.reset();
      this->endResetModel();
   }
   void IdleAnimationFormsModel::_on_form_created(dovah::form_stub& stub) {
      if (stub.form_type != dovah::form_type::idle)
         return;
      if (this->_callback_state.ignore_next_created_idle) {
         this->_callback_state.ignore_next_created_idle = false;
         return;
      }
      this->_datastore.on_idle_created(stub);
   }
   void IdleAnimationFormsModel::_on_form_modified(dovah::form_stub& stub) {
      switch (stub.form_type) {
         case dovah::form_type::action:
            this->_datastore.on_action_modified(stub);
            this->_recache_action(stub);
            break;
         case dovah::form_type::idle:
            //
            // In case the editor ID changed.
            //
            this->_datastore.on_idle_editor_id_potentially_changed(stub);
            this->_recache_idle(stub);
            break;
      }
   }
   void IdleAnimationFormsModel::_on_form_deletion_imminent(dovah::form_stub& stub, bool just_being_flagged) {
      if (!just_being_flagged) {
         if (auto* idle = this->_datastore.idle_by_stub(stub)) {
            this->_drag_and_drop.untrack(*idle);
         }
         this->_datastore.on_before_form_fully_deleted(stub);
         return;
      }
      switch (stub.form_type) {
         case dovah::form_type::action:
            this->_recache_action(stub);
            break;
         case dovah::form_type::idle:
            this->_recache_idle(stub);
            break;
      }
   }
   void IdleAnimationFormsModel::_on_form_deleted(uint32_t form_id, bool just_being_flagged) {
      if (!just_being_flagged)
         return;
      auto& editor = DovahKitCore::get();
      auto* stub   = editor.get_form(form_id);
      if (!stub)
         return;
      switch (stub->form_type) {
         case dovah::form_type::action:
            this->_recache_action(*stub);
            break;
         case dovah::form_type::idle:
            this->_recache_idle(*stub);
            break;
      }
   }
#pragma endregion

void IdleAnimationFormsModel::_rebuild_datastore() {
   this->_cache.clear();
   this->_drag_and_drop.clear();

   auto* flo = DovahKitCore::get().get_file_load_order();
   if (!flo) {
      this->beginResetModel();
      this->_datastore.reset();
      this->endResetModel();
      return;
   }

   this->beginResetModel();
   this->_datastore.build(*flo);
   {
      auto _recache_idle_tree = [this](this auto&& recurse, idle_node& idle) -> void {
         this->_recache_idle(idle);
         for (idle_node* child : idle.child_idles) {
            recurse(*child);
         }
      };

      for (auto* graph : this->_datastore.graphs) {
         auto& cached_graph = this->_cache[graph];
         cached_graph.display_string = QString::fromStdString(graph->path);
         for (action_node* action : graph->actions) {
            this->_recache_action(*action);
            if (idle_node* idle = action->winning_root)
               _recache_idle_tree(*idle);
         }
         for (idle_node* idle : graph->loose->child_idles)
            _recache_idle_tree(*idle);
      }
      for (idle_node* idle : this->_datastore.loose->child_idles)
         _recache_idle_tree(*idle);
   }
   {
      auto& list = this->_datastore.warnings;
      if (!list.empty()) {
         auto& logger = dovahkit::subsystems::message_log::core::get();
         for (const auto* warning : list) {
            QString text;
            if (auto* casted = dynamic_cast<const datastore_warnings::action_root_is_forced_loose*>(warning)) {
               text = tr(
                  "Idle %1 is an action root, but is also flagged as loose, and so risks "
                  "ending up in multiple places at once."
               )
                  .arg(editor_helpers::form_identifiers_to_string(&casted->subject.stub));
            } else if (auto* casted = dynamic_cast<const datastore_warnings::child_idle_is_an_action_root_candidate*>(warning)) {
               text = tr(
                  "Idle %1 has been placed as both an action root and a child idle, and so may "
                  "end up in multiple places at once."
               )
                  .arg(editor_helpers::form_identifiers_to_string(&casted->subject.stub));
            } else if (auto* casted = dynamic_cast<const datastore_warnings::child_of_idle_is_forced_loose*>(warning)) {
               text = tr(
                  "Idle %1 is set to be the child of another idle, but is also flagged as loose, "
                  "so it will not in fact be a child. Is this intentional?"
               )
                  .arg(editor_helpers::form_identifiers_to_string(&casted->subject.stub));
            } else if (auto* casted = dynamic_cast<const datastore_warnings::cyclical_parent_relationships*>(warning)) {
               text = tr("Idle %1's parent chain forms a cyclical reference.")
                  .arg(editor_helpers::form_identifiers_to_string(&casted->subject.stub));
            } else if (auto* casted = dynamic_cast<const datastore_warnings::cyclical_sibling_relationships*>(warning)) {
               text = tr("Idle %1's previous-sibling chain forms a cyclical reference.")
                  .arg(editor_helpers::form_identifiers_to_string(&casted->subject.stub));
            } else if (auto* casted = dynamic_cast<const datastore_warnings::idle_has_candidacies_for_multiple_actions*>(warning)) {
               text = tr(
                  "Idle %1 attempts to be the root idle for multiple actions, and may end up being "
                  "in multiple places at once.\n"
                  "\n"
                  "This can happen if an override attempts to re-parent an action root. More rarely, "
                  "it could happen if a malformed IDLE record contains multiple ANAM subrecords "
                  "placing the same idle in different action roots."
               )
                  .arg(editor_helpers::form_identifiers_to_string(&casted->subject.stub));
            } else if (auto* casted = dynamic_cast<const datastore_warnings::idle_has_multiple_next_siblings*>(warning)) {
               text = tr(
                  "Multiple idles are fighting to have %1 as their previous sibling. This can "
                  "happen if the idle tree has been overridden improperly by a mod."
               )
                  .arg(editor_helpers::form_identifiers_to_string(&casted->subject.stub));
            } else if (auto* casted = dynamic_cast<const datastore_warnings::inconsistent_parentage_on_idle*>(warning)) {
               text = tr("Somehow, idle %1 is not present in its parent's child list.")
                  .arg(editor_helpers::form_identifiers_to_string(&casted->subject.stub));
            } else if (auto* casted = dynamic_cast<const datastore_warnings::loose_idle_is_not_flagged*>(warning)) {
               text = tr("Idle %1 has ended up loose, but wasn't originally flagged as loose. Is this intentional?")
                  .arg(editor_helpers::form_identifiers_to_string(&casted->subject.stub));
            } else if (auto* casted = dynamic_cast<const datastore_warnings::no_loose_idle_list_for_idle*>(warning)) {
               text = tr("There is no loose idle list to place %1 in.")
                  .arg(editor_helpers::form_identifiers_to_string(&casted->subject.stub));
            } else if (auto* casted = dynamic_cast<const datastore_warnings::orphaned_idle*>(warning)) {
               text = tr("Idle %1 is orphaned: it has no parent idle, and isn't an action root.")
                  .arg(editor_helpers::form_identifiers_to_string(&casted->subject.stub));
            } else if (auto* casted = dynamic_cast<const datastore_warnings::previous_sibling_is_not_as_expected*>(warning)) {
               if (auto* node = casted->sibling.intended) {
                  text = tr("After the idle tree was fully built, idle %1 expected to be the next sibling of idle %2.");
                  text = text.arg(editor_helpers::form_identifiers_to_string(&casted->subject.stub));
                  text = text.arg(editor_helpers::form_identifiers_to_string(&node->stub));
               } else {
                  text = tr("After the idle tree was fully built, idle %1 expected to be the first (or possibly only) child of its parent.");
                  text = text.arg(editor_helpers::form_identifiers_to_string(&casted->subject.stub));
               }

               QString instead;
               if (auto* node = casted->sibling.actual) {
                  instead = tr("Instead, the idle is located after %1.");
                  instead = instead.arg(editor_helpers::form_identifiers_to_string(&node->stub));
               } else {
                  instead = tr("Instead, the idle is its parent's first or only child.");
               }

               text = tr("%1 %2", "datastore warning sentence order for 'previous sibling is not as expected'").arg(text).arg(instead);
            } else if (auto* casted = dynamic_cast<const datastore_warnings::sibling_is_an_ancestor*>(warning)) {
               text = tr("Idle %1 has a previous sibling that is one of its ancestor idles.")
                  .arg(editor_helpers::form_identifiers_to_string(&casted->subject.stub));
            } else if (auto* casted = dynamic_cast<const datastore_warnings::siblings_have_mismatched_parents*>(warning)) {
               text = tr("Idle %1 has a different parent from one of its previous siblings.")
                  .arg(editor_helpers::form_identifiers_to_string(&casted->subject.stub));
            } else {
               text = tr(
                  "An unknown warning occurred when loading the idle animation trees. Please contact DovahKit's developer "
                  "so that a warning message can be added for this. If possible, please make backup copies of the file(s) "
                  "that you had loaded at the time, and be ready to send those along, so the developer can reproduce the "
                  "problem on their end."
               );
            }
            logger.addLogItem({
               text,
               ui::types::log_item_type::warning,
               ui::types::log_item_context::unspecified
            });
         }
      }
   }
   this->endResetModel();
}
void IdleAnimationFormsModel::_recache_action(const action_node& node) {
   auto& stub  = node.stub;
   auto& cache = this->_cache[(action_node*)&node];
   cache.display_string = QString::fromStdString(stub.get_editor_id());
   if (stub.is_edited()) {
      cache.display_string += tr(" *", "form indicator: edited");
   }
   if (stub.is_deleted()) {
      cache.display_string += tr(" (D)", "form indicator: deletion");
   }
}
void IdleAnimationFormsModel::_recache_action(const dovah::form_stub& stub) {
   for (auto* graph : this->_datastore.graphs) {
      auto index = graph->index_of_action(stub);
      if (index == datastore_node::index_of_none)
         continue;
      this->_recache_action(*graph->actions[index]);
      auto tl = _qmi_for_child_node(index, 0, *graph);
      auto br = _qmi_for_child_node(index, this->columnCount({}), *graph);
      emit dataChanged(tl, br);
   }
}
void IdleAnimationFormsModel::_recache_idle(const idle_node& node) {
   auto& stub  = node.stub;
   auto& cache = this->_cache[(idle_node*)&node];
   cache.display_string = QString::fromStdString(stub.get_editor_id());
   if (stub.is_edited()) {
      cache.display_string += tr(" *", "form indicator: edited");
   }
   if (stub.is_deleted()) {
      cache.display_string += tr(" (D)", "form indicator: deletion");
   }
}
void IdleAnimationFormsModel::_recache_idle(const dovah::form_stub& stub) {
   auto* node = this->_datastore.idle_by_stub(stub);
   if (node) {
      this->_recache_idle(*node);
      auto tl = _qmi_for_node(*node);
      auto br = _qmi_for_node(*node, this->columnCount({}) - 1);
      emit dataChanged(tl, br);
   }
}

#pragma region Form utils
   dovah::form_stub* IdleAnimationFormsModel::_try_silently_create_idle(QString editor_id) {
      //
      // Normally, we automatically react to the creation of an IDLE form occurring 
      // anywhere in DovahKit: we tell the datastore that a new IDLE has been created, 
      // and in turn, the datastore's own callbacks lead back to us invoking callbacks 
      // on QAbstractItemModel for when a row is inserted. This is sufficient for when 
      // IDLEs are created outside of our control.
      // 
      // The IDLE is created "bare," with no parent and no behavior graph, so the row 
      // would be inserted into the model-level loose idles. From there, we could then 
      // trigger the IDLE to be moved into the correct place, when we're the ones who 
      // created the IDLE.
      // 
      // However, it's cleaner for us to have finer-grained control over this process. 
      // Since we can't slip in between the form being created and it being configured, 
      // the better option is:
      // 
      //  - Set a flag so that we ignore the next form-creation notification that we 
      //    get from DovahKitCore, such that we don't tell the datastore about the IDLE 
      //    we're creating.
      // 
      //  - Create the `idle_node` ourselves.
      // 
      //  - Use the datastore to insert the node. Since it has no parent node (not even 
      //    the model-level loose idle container node), the datastore will trigger the 
      //    insertion callbacks rather than the movement callbacks.
      // 
      // This function handles the first of those three steps for the specific case of 
      // duplicating an idle.
      //
      this->_callback_state.ignore_next_created_idle = true;
      auto request = DovahKitCore::get().request_form_creation(dovah::form_type::idle);
      request.editorID = editor_id.toStdString();
      try {
         auto* stub = request.commit();
         if (!stub) {
            this->_callback_state.ignore_next_created_idle = false;
         }
         return stub;
      } catch (...) {
         this->_callback_state.ignore_next_created_idle = false;
         throw;
      }
   }
   dovah::form_stub* IdleAnimationFormsModel::_try_silently_duplicate_idle(dovah::form_stub& idle) {
      //
      // See documentation comment in `_try_silently_create_idle`.
      //

      auto& editor = DovahKitCore::get();

      dovah::form_stub* stub = nullptr;

      this->_callback_state.ignore_next_created_idle = true;
      try {
         auto request = editor.request_form_duplication();
         request.set_target(&idle);
         request.editorID = editor_helpers::make_editor_id_for_duplicate(idle.get_editor_id()).toStdString();
         stub = request.commit();
      } catch (...) {
         this->_callback_state.ignore_next_created_idle = false;
         throw;
      }
      if (!stub) {
         this->_callback_state.ignore_next_created_idle = false;
         return nullptr;
      }
      return stub;
   }
#pragma endregion
#pragma region Node utils
   IdleAnimationFormsModel::action_node& IdleAnimationFormsModel::_get_or_create_action(graph_node& graph, dovah::form_stub& action) {
      assert(action.form_type == dovah::form_type::action);
      auto* a_node = graph.get_action(action);
      if (a_node)
         return *a_node;
      
      auto qmi = _qmi_for_node(graph);
      auto i   = graph.prospective_index_of(action);
      this->beginInsertRows(qmi, i, i);
      {
         auto a_node_ptr = std::make_unique<action_node>(this->_datastore, action);
         a_node = a_node_ptr.get();
         graph.actions.insert(graph.actions.begin() + i, a_node);
         a_node->graph = &graph;
         a_node_ptr.release();
         this->_recache_action(*a_node);
      }
      this->endInsertRows();
      //
      return *a_node;
   }
   IdleAnimationFormsModel::idle_node* IdleAnimationFormsModel::_create_action_root(graph_node& graph, dovah::form_stub& action, QString idle_editor_id) {
      dovah::form_stub* idle_stub = this->_try_silently_create_idle(idle_editor_id);
      if (!idle_stub)
         return nullptr;

      auto& a_node     = _get_or_create_action(graph, action);
      auto  i_node_ptr = std::make_unique<idle_node>(this->_datastore, *idle_stub);
      auto* i_node     = i_node_ptr.get();
      assert(i_node != nullptr);
      this->_datastore.move_idle(*i_node, a_node, nullptr);
      i_node_ptr.release();
      return i_node;
   }

   bool IdleAnimationFormsModel::_could_ever_move_idles_into(const datastore_node& destination) const {
      if (dynamic_cast<const graph_node*>(&destination))
         return false;
      //
      // Don't allow moving into an action.
      //
      if (dynamic_cast<const action_node*>(&destination))
         return false;
      //
      return true;
   }
   bool IdleAnimationFormsModel::_can_move_idle_into(const idle_node& subject, const datastore_node& destination) const {
      assert(_could_ever_move_idles_into(destination)); // require that the caller have checked this first
      return this->_datastore.is_idle_movement_legal(subject, destination, nullptr) && !this->_datastore.is_idle_movement_a_really_bad_idea(subject, destination, nullptr);
   }
   void IdleAnimationFormsModel::_unchecked_move_idle_node(idle_node& subject, datastore_node& destination, int row) {
      assert(_can_move_idle_into(subject, destination));
      auto subject_qmi     = _qmi_for_node(subject);
      auto destination_qmi = _qmi_for_node(destination);

      idle_node* previous = nullptr;
      if (row < 0) {
         if (auto* dst_idle = dynamic_cast<idle_node*>(&destination))
            if (!dst_idle->child_idles.empty())
               previous = dst_idle->child_idles.back();
      } else if (row > 0) {
         if (auto* dst_idle = dynamic_cast<idle_node*>(&destination))
            previous = dst_idle->child_idles[row - 1];
      }

      // the datastore fires callbacks that trigger beforeMoveRows/endMoveRows when 
      // we tell it to move the node, so we shouldn't fire those here.

      this->_datastore.move_idle(subject, destination, previous);
   }
#pragma endregion

#pragma region Accessors
   QModelIndex IdleAnimationFormsModel::graphQMI(QString path) const noexcept {
      auto* node = this->_node_for_graph_path(path);
      if (!node)
         return {};
      return _qmi_for_node(*node);
   }
   QModelIndex IdleAnimationFormsModel::idleQMI(dovah::form_stub& stub) const noexcept {
      if (stub.form_type != dovah::form_type::idle)
         return {};
      auto* node = this->_datastore.idle_by_stub(stub);
      if (!node)
         return {};
      return _qmi_for_node(*node);
   }

   QModelIndex IdleAnimationFormsModel::getOrCreateGraph(QString path) {
      auto* graph = this->_datastore.get_or_create_graph(path.toStdString());
      if (!graph)
         return {};

      auto& cached_graph = this->_cache[graph];
      cached_graph.display_string = QString::fromStdString(graph->path);

      return _qmi_for_node(*graph);
   }

   [[nodiscard]] std::vector<dovah::form_stub*> IdleAnimationFormsModel::_actionsByGraph(const graph_node& graph) const noexcept {
      std::vector<dovah::form_stub*> stubs;
      for (const action_node* action : graph.actions) {
         stubs.push_back(&action->stub);
      }
      return stubs;
   }
   [[nodiscard]] std::vector<dovah::form_stub*> IdleAnimationFormsModel::actionsByGraph(const QModelIndex& qmi) const noexcept {
      if (!qmi.isValid())
         return {};
      const datastore_node* node  = _node_for_qmi(qmi);
      const graph_node*     graph = dynamic_cast<const graph_node*>(node);
      if (!graph)
         return {};
      return this->_actionsByGraph(*graph);
   }
   [[nodiscard]] std::vector<dovah::form_stub*> IdleAnimationFormsModel::actionsByGraph(QString path) const noexcept {
      auto* graph = _node_for_graph_path(path);
      if (!graph)
         return {};
      return this->_actionsByGraph(*graph);
   }

   QModelIndex IdleAnimationFormsModel::createActionRoot(const QModelIndex& graph_qmi, dovah::form_stub& action, QString idle_editor_id) {
      if (!graph_qmi.isValid())
         return {};
      auto* graph = dynamic_cast<graph_node*>(_node_for_qmi(graph_qmi));
      if (!graph)
         return {};
      auto* node = this->_create_action_root(*graph, action, idle_editor_id);
      if (!node)
         return {};
      return _qmi_for_node(*node);
   }
   QModelIndex IdleAnimationFormsModel::createActionRoot(QString graph_path, dovah::form_stub& action, QString idle_editor_id) {
      auto* graph = _node_for_graph_path(graph_path);
      if (!graph)
         return {};
      auto* node = this->_create_action_root(*graph, action, idle_editor_id);
      if (!node)
         return {};
      return _qmi_for_node(*node);
   }

   bool IdleAnimationFormsModel::canCreateIdleIn(const QModelIndex& parent) const {
      auto* node = _node_for_qmi(parent);
      if (!node)
         return false;
      return _can_create_new_idle_in(*node);
   }
   QModelIndex IdleAnimationFormsModel::createIdle(const QModelIndex& parent_qmi, QString idle_editor_id) {
      datastore_node* parent_node = _node_for_qmi(parent_qmi);
      if (!parent_node)
         return {};
      if (!_can_create_new_idle_in(*parent_node))
         return {};

      dovah::form_stub* stub = this->_try_silently_create_idle(idle_editor_id);
      if (!stub)
         return {};

      idle_node* previous = nullptr;
      if (auto* dst_idle = dynamic_cast<idle_node*>(parent_node))
         if (!dst_idle->child_idles.empty())
            previous = dst_idle->child_idles.back();

      auto  i_node_ptr = std::make_unique<idle_node>(this->_datastore, *stub);
      auto* i_node     = i_node_ptr.get();
      assert(i_node != nullptr);
      this->_datastore.move_idle(*i_node, *parent_node, previous);
      assert(i_node->canonical_parent != nullptr);
      i_node_ptr.release();
      return _qmi_for_node(*i_node);
   }

   bool IdleAnimationFormsModel::canEverDuplicateIdle(const QModelIndex& idle_qmi) const {
      auto* src_node = _node_for_qmi(idle_qmi);
      if (!src_node)
         return false;
      auto* src_idle = dynamic_cast<const idle_node*>(src_node);
      if (!src_idle) // QMI is not that of an idle
         return false;
      return _can_ever_duplicate(*src_idle);
   }
   QModelIndex IdleAnimationFormsModel::duplicateIdle(const QModelIndex& idle_qmi, bool and_descendants) {
      auto* src_node = _node_for_qmi(idle_qmi);
      if (!src_node)
         return {};
      auto* src_idle = dynamic_cast<idle_node*>(src_node);
      if (!src_idle) // QMI is not that of an idle
         return {};

      if (!_can_ever_duplicate(*src_idle))
         return {};

      auto& editor = DovahKitCore::get();
      if (!editor.has_data())
         return {};

      if (and_descendants) {
         //
         // Ensure there are sufficient form IDs available in the active file.
         //
         {
            auto* flo = editor.get_file_load_order();
            if (!flo)
               return {};

            size_t count_to_duplicate = [](this auto&& recurse, idle_node& idle) -> size_t {
               size_t count = 1;
               for (auto& child_ptr : idle.child_idles) {
                  count += recurse(*child_ptr);
               }
               return count;
            }(*src_idle);

            dovah::bare_form_id_t last_found_form_id     = 0;
            bool                  all_form_ids_available = true;
            size_t count_available = 0;
            for (size_t i = 0; i < count_to_duplicate; ++i) {
               auto id = flo->find_first_free_form_id_in_active_file(last_found_form_id);
               if (id == 0) {
                  all_form_ids_available = false;
                  break;
               }
               ++count_available;
               last_found_form_id = id;
            }
            if (!all_form_ids_available) {
               throw too_many_to_duplicate_exception(count_to_duplicate, count_available);
            }
         }
         //
         // Recursively duplicate the IDLEs.
         //
         assert(src_idle->canonical_parent);
         QModelIndex root_qmi = {};
         [this, &root_qmi](this auto&& recurse, idle_node& idle, datastore_node& dst_parent, bool is_root = false) -> void {
            dovah::form_stub* stub = this->_try_silently_duplicate_idle(idle.stub);
            if (!stub)
               return;

            auto  i_node_ptr = std::make_unique<idle_node>(this->_datastore, *stub);
            auto* i_node = i_node_ptr.get();
            assert(i_node != nullptr);
            if (is_root) {
               this->_datastore.move_idle(
                  *i_node,
                  dst_parent,
                  &idle
               );
            } else {
               idle_node* parent_idle = dynamic_cast<idle_node*>(&dst_parent);
               assert(parent_idle != nullptr);
               idle_node* previous_idle = nullptr;
               if (!parent_idle->child_idles.empty())
                  previous_idle = parent_idle->child_idles.back();
               this->_datastore.move_idle(
                  *i_node,
                  dst_parent,
                  previous_idle
               );
            }
            i_node_ptr.release();
            if (is_root)
               root_qmi = _qmi_for_node(*i_node);

            for (auto& child_ptr : idle.child_idles) {
               recurse(*child_ptr, *i_node);
            }
         }(*src_idle, *src_idle->canonical_parent, true);
         return root_qmi;
      } else {
         dovah::form_stub* stub = this->_try_silently_duplicate_idle(src_idle->stub);
         if (!stub)
            return {};

         auto  i_node_ptr = std::make_unique<idle_node>(this->_datastore, *stub);
         auto* i_node = i_node_ptr.get();
         assert(i_node != nullptr);
         assert(src_idle->canonical_parent);
         this->_datastore.move_idle(
            *i_node,
            *src_idle->canonical_parent,
            src_idle
         );
         i_node_ptr.release();
         return _qmi_for_node(*i_node);
      }
   }

   bool IdleAnimationFormsModel::canDeleteIdle(const QModelIndex& idle_qmi) const {
      auto* src_node = _node_for_qmi(idle_qmi);
      if (!src_node)
         return false;
      auto* src_idle = dynamic_cast<const idle_node*>(src_node);
      if (!src_idle) // QMI is not that of an idle
         return false;

      return this->_datastore.is_idle_deletion_legal(*src_idle) && !this->_datastore.is_idle_deletion_a_really_bad_idea(*src_idle);
   }
   void IdleAnimationFormsModel::deleteIdle(const QModelIndex& idle_qmi, QWidget* error_dialog_parent) {
      auto* src_node = _node_for_qmi(idle_qmi);
      if (!src_node)
         return;
      auto* src_idle = dynamic_cast<idle_node*>(src_node);
      if (!src_idle) // QMI is not that of an idle
         return;

      this->_handler_state.any_deletions_failed = false;
      this->_datastore.delete_idle(*src_idle);
      if (this->_handler_state.any_deletions_failed) {
         this->_handler_state.any_deletions_failed = false;
         QMessageBox::critical(
            error_dialog_parent,
            QObject::tr("Error", "delete form error"),
            QObject::tr("Unable to delete all of the needed idles.")
         );
      }
   }

   bool IdleAnimationFormsModel::canMoveIdleUp(const QModelIndex& idle_qmi) const {
      if (!idle_qmi.isValid())
         return false;
      auto* idle = dynamic_cast<const idle_node*>(_node_for_qmi(idle_qmi));
      if (!idle)
         return false;
      auto* parent = dynamic_cast<const idle_node*>(idle->canonical_parent);
      if (!parent)
         return false;

      size_t i = parent->index_of_child(*idle);
      assert(i != datastore_node::index_of_none);
      return i > 0;
   }
   bool IdleAnimationFormsModel::canMoveIdleDown(const QModelIndex& idle_qmi) const {
      if (!idle_qmi.isValid())
         return false;
      auto* idle = dynamic_cast<const idle_node*>(_node_for_qmi(idle_qmi));
      if (!idle)
         return false;
      auto* parent = dynamic_cast<const idle_node*>(idle->canonical_parent);
      if (!parent)
         return false;

      size_t i = parent->index_of_child(*idle);
      assert(i != datastore_node::index_of_none);
      return i < parent->child_idles.size() - 1;
   }
   void IdleAnimationFormsModel::moveIdleUp(const QModelIndex& idle_qmi) {
      auto* idle = dynamic_cast<idle_node*>(_node_for_qmi(idle_qmi));
      if (!idle)
         return;
      this->_datastore.move_idle_within_parent(*idle, -1);
   }
   void IdleAnimationFormsModel::moveIdleDown(const QModelIndex& idle_qmi) {
      auto* idle = dynamic_cast<idle_node*>(_node_for_qmi(idle_qmi));
      if (!idle)
         return;
      this->_datastore.move_idle_within_parent(*idle, 1);
   }

   bool IdleAnimationFormsModel::isNonCanonicalPosition(const QModelIndex& idle_qmi) const noexcept {
      const auto* via_parent = (const datastore_node*)idle_qmi.internalPointer();
      const auto* node       = _node_for_qmi(idle_qmi);
      if (auto* casted = dynamic_cast<const idle_node*>(node)) {
         return casted->canonical_parent != via_parent;
      }
      return false;
   }
   QModelIndex IdleAnimationFormsModel::canonicalPosition(const QModelIndex& src_qmi) const noexcept {
      const auto* node = dynamic_cast<const idle_node*>(_node_for_qmi(src_qmi));
      if (!node)
         return src_qmi;
      return _qmi_for_node(*node);
   }
#pragma endregion

#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex IdleAnimationFormsModel::index(int row, int column, const QModelIndex& parent_qmi) const /*override*/ {
         if (row < 0 || column < 0)
            return {};
         const datastore_node* parent_node = _node_for_qmi(parent_qmi);
         if (auto* parent_action = dynamic_cast<const action_node*>(parent_node)) {
            if (row != 0)
               return {};
            //
            // Idles can potentially be in multiple places at once: an arbitrary number of 
            // parent actions plus at most one parent idle. When an idle is in multiple 
            // places at once, one of those places is considered "canonical." If one of 
            // those places is an idle, then that must be the canonical parent.
            // 
            // When an idle is in multiple places at once, the QMI we use for it depends 
            // on how we got to the idle, i.e. the parent through which we accessed it, 
            // since QMIs consist of a parent pointer and child index.
            //
            if (parent_action->winning_root) {
               return _qmi_for_child_node(row, column, *parent_action);
            }
            return {};
         }
         const datastore_node* target_node = _child_node_by_row(parent_node, row);
         if (!target_node)
            return {};
         return _qmi_for_node(*target_node, column);
      }
      /*virtual*/ QModelIndex IdleAnimationFormsModel::parent(const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         const datastore_node* subject = _node_for_qmi(index);
         const datastore_node* parent  = nullptr;
         if (auto* casted = dynamic_cast<const action_node*>(subject)) {
            parent = casted->graph;
         } else if (auto* casted = dynamic_cast<const idle_node*>(subject)) {
            //
            // Idles can potentially be in multiple places at once, so we have to 
            // rely on the parent pointer in the QMI.
            //
            parent = (datastore_node*)index.internalPointer();
         } else if (auto* casted = dynamic_cast<const loose_idle_list_node*>(subject)) {
            parent = casted->graph;
         }
         if (parent)
            return _qmi_for_node(*parent);
         return {};
      }
      /*virtual*/ QModelIndex IdleAnimationFormsModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
         if (row < 0 || column < 0)
            return {};
         const auto* basis = _node_for_qmi(index);
         bool basis_is_top_level = false;
         if (!basis)
            return {};
         if (auto* casted = dynamic_cast<const action_node*>(basis)) {
            if (!casted->graph)
               return {};
            if (!_child_node_by_row(casted->graph, row))
               return {};
            return _qmi_for_child_node(row, column, *casted->graph);
         } else if (auto* casted = dynamic_cast<const idle_node*>(basis)) {
            //
            // Idles can potentially be in multiple places at once, so we have to 
            // rely on the parent pointer in the QMI.
            //
            auto* parent = (datastore_node*)index.internalPointer();
            if (dynamic_cast<action_node*>(parent)) {
               if (row != 0)
                  return {};
            } else if (auto* casted_parent = dynamic_cast<idle_node*>(parent)) {
               if (row >= casted_parent->child_idles.size())
                  return {};
            } else if (auto* casted_parent = dynamic_cast<loose_idle_list_node*>(parent)) {
               if (row >= casted_parent->child_idles.size())
                  return {};
            }
            return _qmi_for_child_node(row, column, *parent);
         } else if (dynamic_cast<const graph_node*>(basis)) {
            basis_is_top_level = true;
         } else if (auto* casted = dynamic_cast<const loose_idle_list_node*>(basis)) {
            if (auto* graph = casted->graph) {
               if (row <= graph->actions.size())
                  return _qmi_for_child_node(row, column, *graph);
               return {};
            }
            basis_is_top_level = true; // model-level LOOSE node
         }
         if (basis_is_top_level) {
            const size_t size = this->_datastore.graphs.size();
            if (row < size)
               return _qmi_for_node(*this->_datastore.graphs[row], column);
            datastore_node* subject = nullptr;
            if (row == size) {
               auto* loose = this->_datastore.loose;
               if (loose && !loose->child_idles.empty())
                  subject = loose;
            }
            if (subject)
               return _qmi_for_node(*subject, column);
         }
         return {};
      }
      /*virtual*/ int IdleAnimationFormsModel::rowCount(const QModelIndex& parent) const /*override*/ {
         const auto* node = _node_for_qmi(parent);
         if (!node) {
            size_t size = this->_datastore.graphs.size();
            const auto* loose = this->_datastore.loose;
            if (loose && !loose->child_idles.empty())
               ++size;
            return size;
         }
         size_t size = 0;
         if (auto* casted = dynamic_cast<const graph_node*>(node)) {
            size = casted->actions.size() + 1; // +1 for loose
         } else if (auto* casted = dynamic_cast<const idle_node*>(node)) {
            if (casted->canonical_parent != parent.internalPointer()) {
               //
               // This idle is present in multiple places at once, and this particular place 
               // isn't the idle's canonical parent. Do not allow expanding the idle and 
               // viewing its child nodes from this particular place (QMIs can't properly 
               // represent children of a node that exists in multiple spots in a tree).
               // 
               return 0;
            }
            size = casted->child_idles.size();
         } else if (auto* casted = dynamic_cast<const loose_idle_list_node*>(node)) {
            size = casted->child_idles.size();
         } else if (auto* casted = dynamic_cast<const action_node*>(node)) {
            if (casted->winning_root)
               size = 1;
         }
         return size;
      }
      /*virtual*/ int IdleAnimationFormsModel::columnCount(const QModelIndex& parent) const /*override*/ {
         return 1;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant IdleAnimationFormsModel::data(const QModelIndex& qmi, int role) const /*override*/ {
         const datastore_node* node = _node_for_qmi(qmi);
         if (!node)
            return {};
         switch (role) {
            case NodeTypeRole:
               if (dynamic_cast<const graph_node*>(node))
                  return QVariant::fromValue(NodeType::Graph);
               if (dynamic_cast<const action_node*>(node))
                  return QVariant::fromValue(NodeType::Action);
               if (dynamic_cast<const idle_node*>(node))
                  return QVariant::fromValue(NodeType::Idle);
               if (auto* loose = dynamic_cast<const loose_idle_list_node*>(node)) {
                  if (!loose->graph)
                     return QVariant::fromValue(NodeType::LooseIdlesPerModel);
                  return QVariant::fromValue(NodeType::LooseIdlesPerGraph);
               }
               break;
            case FormStubRole:
               if (auto* casted = dynamic_cast<const action_node*>(node))
                  return QVariant::fromValue(&casted->stub);
               if (auto* casted = dynamic_cast<const idle_node*>(node))
                  return QVariant::fromValue(&casted->stub);
               break;
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
               {
                  auto it = this->_cache.find((datastore_node*)node); // can't use a const pointer to look up a non-const-pointer key -_-
                  if (it != this->_cache.end()) {
                     return it->second.display_string;
                  }
               }
               if (node == this->_datastore.loose)
                  return tr("LOOSE");
               if (dynamic_cast<const loose_idle_list_node*>(node))
                  return tr("LOOSE");
               break;
            case Qt::ForegroundRole:
               if (dynamic_cast<const action_node*>(node)) {
                  return QColor(64, 160, 255);
               }
               if (auto* idle = dynamic_cast<const idle_node*>(node)) {
                  if (idle->canonical_parent != qmi.internalPointer()) {
                     //
                     // This idle is present in multiple places at once, and this particular place 
                     // isn't the idle's canonical parent.
                     // 
                     return QColor(255, 0, 0);
                  }
               }
               break;
         }
         return {};
      }
      /*virtual*/ Qt::ItemFlags IdleAnimationFormsModel::flags(const QModelIndex& index) const /*override*/ {
         auto  flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled;
         auto* node  = _node_for_qmi(index);
         if (!node) {
            return flags;
         }

         {
            bool can_drag = false;
            if (auto* casted_idle = dynamic_cast<const idle_node*>(node)) {
               can_drag = true;
               if (dynamic_cast<const action_node*>(casted_idle->canonical_parent)) {
                  //
                  // Don't allow drag-moving an action root.
                  //
                  can_drag = false;
               }
            }
            if (can_drag)
               flags |= Qt::ItemIsDragEnabled;
         }

         {
            bool can_drop = true;
            if (auto* action = dynamic_cast<const action_node*>(node)) {
               can_drop = action->winning_root == nullptr;
            } else if (auto* idle = dynamic_cast<const idle_node*>(node)) {
               //
               // Don't allow dropping an idle into a loose idle, unless that loose idle already 
               // has child idles for some reason.
               //
               if (idle->child_idles.empty())
                  if (dynamic_cast<const loose_idle_list_node*>(idle->canonical_parent))
                     can_drop = false;
            }
            if (can_drop)
               flags |= Qt::ItemIsDropEnabled;
         }

         return flags;
      }
   #pragma endregion
   /*virtual*/ QVariant IdleAnimationFormsModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
      if (orientation == Qt::Orientation::Horizontal && section == 0) {
         switch (role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
               return tr("Animations");
         }
      }
      return {};
   }
   #pragma region Drag and drop
      #pragma region Whole-model queries
         /*virtual*/ QStringList IdleAnimationFormsModel::mimeTypes() const /*override*/ {
            return { QString::fromLatin1(mime_type) };
         }
         /*virtual*/ Qt::DropActions IdleAnimationFormsModel::supportedDropActions() const /*override*/ {
            return Qt::DropAction::CopyAction | Qt::DropAction::MoveAction;
         }
      #pragma endregion
      /*virtual*/ QMimeData* IdleAnimationFormsModel::mimeData(const QModelIndexList& indices) const /*override*/ {
         if (indices.count() <= 0)
            return nullptr;
         QByteArray  data;
         QDataStream stream(&data, QIODevice::WriteOnly);
         //
         // Stream begins with our `this` pointer. The pointer is used only for equality 
         // checks on drop (i.e. no moving/copying procedure data across packages) and is 
         // never dereferenced.
         //
         stream << (intptr_t)this;
         //
         for (const QModelIndex& qmi : indices) {
            const auto* node = _node_for_qmi(qmi);
            if (!node)
               continue;
            const auto* casted = dynamic_cast<const idle_node*>(node);
            if (!casted)
               continue;
            //
            // QAbstractItemModel's default implementation serializes all itemData for the 
            // node into the stream. We can't do that because some things, like conditions, 
            // are both not serializable *and* require references to objects held elsewhere. 
            // If QAbstractItemModel's implementation is designed to avoid the possibility 
            // of referenced objects being deleted during the drag operation, then trying 
            // to serialize conditions fails that requirement.
            // 
            // We also can't track the lifetime of the drag operation, so we can't, for 
            // example, store a map of unique IDs to QPersistentModelIndexes, because we 
            // wouldn't know when to destroy the QPMIs.
            // 
            // Our solution is to create IDs on demand for nodes that are being dragged, 
            // and serialize those. We never expose direct access to nodes (and thus to the 
            // unique_ptr list of child nodes), so all node removals go through us; we can 
            // invalidate unique IDs properly.
            // 
            // Of course, since we can't track the lifetime of a drag operation, we have to 
            // assume that a request for a node's MIME data is the start of a drag, and we 
            // have to create the ID then. Since the mimeData() getter is const, this is... 
            // a complication.
            //
            stream << this->_drag_and_drop.track(*const_cast<idle_node*>(casted));
         }
         //
         QMimeData* mime = new QMimeData();
         mime->setData(mime_type, data);
         return mime;
      }
      /*virtual*/ bool IdleAnimationFormsModel::canDropMimeData(const QMimeData* mime, Qt::DropAction action, int row, int column, const QModelIndex& parent) const /*override*/ {
         QByteArray  data = mime->data(mime_type);
         QDataStream stream(&data, QIODevice::ReadOnly);
         {  // Verify that this is an internal move.
            std::intptr_t this_pointer;
            stream >> this_pointer;
            if ((IdleAnimationFormsModel*)this_pointer != this)
               return false;
         }

         if (parent == _qmi_for_model_root())
            //
            // Don't allow movement of a non-top-level node to the top level.
            //
            return false;

         const auto* parent_node = _node_for_qmi(parent);
         if (!parent_node) // sanity
            return false;
         if (!_could_ever_move_idles_into(*parent_node))
            return false;

         std::vector<idle_node*> dragged_nodes;
         while (!stream.atEnd()) {
            DragDropTracking::uid_t id;
            stream >> id;
            auto* node = this->_drag_and_drop.get_by_id(id);
            if (node)
               dragged_nodes.push_back(node);
         }
         if (!dragged_nodes.size())
            return true;

         for (auto* dragged : dragged_nodes)
            if (!_can_move_idle_into(*dragged, *parent_node))
               return false;

         return true;
      }
      /*virtual*/ bool IdleAnimationFormsModel::dropMimeData(const QMimeData* mime, Qt::DropAction action, int row, int column, const QModelIndex& parent) /*override*/ {
         if (!this->canDropMimeData(mime, action, row, column, parent))
            return false;

         QByteArray  data = mime->data(mime_type);
         QDataStream stream(&data, QIODevice::ReadOnly);
         {  // Verify that this is an internal move.
            std::intptr_t this_pointer;
            stream >> this_pointer;
            if ((IdleAnimationFormsModel*)this_pointer != this)
               return false;
         }
         std::vector<idle_node*> nodes;
         while (!stream.atEnd()) {
            DragDropTracking::uid_t id;
            stream >> id;
            auto* node = this->_drag_and_drop.get_by_id(id);
            if (node)
               nodes.push_back(node);
         }
         if (!nodes.size())
            return false;

         auto* destination_parent = _node_for_qmi(parent);
         assert(destination_parent != nullptr);
         for (auto it = nodes.rbegin(); it != nodes.rend(); ++it)
            this->_unchecked_move_idle_node(**it, *destination_parent, row);
         return true;
      }
   #pragma endregion
#pragma endregion

#pragma region IdleAnimationFormsModel::DragDropTracking
   IdleAnimationFormsModel::DragDropTracking::uid_t IdleAnimationFormsModel::DragDropTracking::track(idle_node& node) {
      for (const auto& pair : this->nodes)
         if (pair.second == &node)
            return pair.first;
      auto id = this->next_id;
      this->next_id++;
      this->nodes[id] = &node;
      return id;
   }
   void IdleAnimationFormsModel::DragDropTracking::untrack(idle_node& node) {
      auto& map = this->nodes;
      auto  it  = std::find_if(map.begin(), map.end(), [&node](const auto& pair) {
         return pair.second == &node;
      });
      if (it != map.end())
         map.erase(it);
   }
   void IdleAnimationFormsModel::DragDropTracking::clear() {
      this->nodes.clear();
   }
   IdleAnimationFormsModel::idle_node* IdleAnimationFormsModel::DragDropTracking::get_by_id(uid_t id) {
      auto& map = this->nodes;
      auto  it  = map.find(id);
      if (it != map.end())
         return it->second;
      return nullptr;
   }
#pragma endregion