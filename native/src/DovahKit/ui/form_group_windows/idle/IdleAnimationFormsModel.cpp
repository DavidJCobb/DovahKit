#include "./IdleAnimationFormsModel.h"
#include <QColor>
#include <QMessageBox>
#include "dovah/datastores/idles/action_node.h"
#include "dovah/datastores/idles/graph_node.h"
#include "dovah/datastores/idles/idle_node.h"
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
#include "dovah/datastores/idles/warnings/cyclical_parent_relationships.h"
#include "dovah/datastores/idles/warnings/cyclical_sibling_relationships.h"
#include "dovah/datastores/idles/warnings/idle_has_multiple_next_siblings.h"
#include "dovah/datastores/idles/warnings/inconsistent_parentage.h"
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
   #pragma region Set up datastore callbacks
      #pragma region Global
         {
            auto& cb_set = this->_datastore.callbacks.on_cleared;
            cb_set.before = [this]() { this->beginResetModel(); };
            cb_set.after = [this]() { this->endResetModel(); };
         }
         {
            auto& cb_set = this->_datastore.callbacks.on_any_form_modified;
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
            auto& cb_set = this->_datastore.callbacks.graphs.on_created;
            cb_set.before = [this](std::string_view path, size_t index) {
               this->beginInsertRows(_qmi_for_model_root(), index, index);
            };
            cb_set.after = [this](std::string_view path, const graph_node&) {
               this->endInsertRows();
            };
         }
      #pragma endregion
      #pragma region Actions
         {
            auto& cb_set = this->_datastore.callbacks.actions.on_deleted;
            cb_set.before = [this](const action_node& node) {
               if (node.parent) {
                  auto qmi = _qmi_for_node(*node.parent);
                  auto i   = node.parent->index_of_child(node);
                  this->beginRemoveRows(qmi, i, i);
                  this->_callback_state.emitted_last_deletion = true;
               }
            };
            cb_set.after = [this](const dovah::form_stub&) {
               if (this->_callback_state.emitted_last_deletion)
                  this->endRemoveRows();
               this->_callback_state.emitted_last_deletion = false;
            };
         }
         {
            auto& cb_set = this->_datastore.callbacks.actions.on_placed;
            cb_set.before = [this](const action_node& node, const action_parent_node& parent_after, size_t index) {
               if (node.parent) {
                  auto& parent_prior = *node.parent;
                  auto  from         = parent_prior.index_of_child(node);
                  auto  to           = index;
                  auto  qmi_prior = _qmi_for_node(parent_prior);
                  auto  qmi_after = _qmi_for_node(parent_after);
                  if (to >= from)
                     ++to;
                  this->_callback_state.last_node_placement_was_an_insertion = false;
                  this->beginMoveRows(qmi_prior, from, from, qmi_after, to);
               } else {
                  this->_callback_state.last_node_placement_was_an_insertion = true;
                  this->beginInsertRows(_qmi_for_node(parent_after), index, index);
               }
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
            auto& cb_set = this->_datastore.callbacks.idles.on_deleted;
            cb_set.before = [this](const idle_node& node) {
               if (node.parent) {
                  auto qmi = _qmi_for_node(*node.parent);
                  auto i   = node.parent->index_of_child(node);
                  this->beginRemoveRows(qmi, i, i);
                  this->_callback_state.emitted_last_deletion = true;
               }
            };
            cb_set.after = [this](const dovah::form_stub&) {
               if (this->_callback_state.emitted_last_deletion)
                  this->endRemoveRows();
               this->_callback_state.emitted_last_deletion = false;
            };
         }
         {
            auto& cb_set = this->_datastore.callbacks.idles.on_placed;
            cb_set.before = [this](const idle_node& node, const idle_parent_node& parent_after, size_t index) {
               if (node.parent) {
                  auto& parent_prior = *node.parent;
                  auto  from         = parent_prior.index_of_child(node);
                  auto  to           = index;
                  auto  qmi_prior = _qmi_for_node(parent_prior);
                  auto  qmi_after = _qmi_for_node(parent_after);
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

#pragma region Node/QMI utils and node lookups
   QModelIndex IdleAnimationFormsModel::_qmi_for_model_root() const {
      return {};
   }
   QModelIndex IdleAnimationFormsModel::_qmi_for_node(const datastore_node& n, int column) const {
      if (column < 0)
         return {};
      const auto& graphs = this->_datastore.graphs;
      const auto  g_size = graphs.size();

      {
         auto* loose_a = this->_datastore.loose.actions;
         auto* loose_i = this->_datastore.loose.idles;
         if (&n == loose_a) {
            return this->createIndex(g_size, column, nullptr);
         }
         if (&n == loose_i) {
            if (loose_a && !loose_a->children.empty())
               return this->createIndex(g_size + 1, column, nullptr);
            return this->createIndex(g_size, column, nullptr);
         }
      }

      const void* parent_void = nullptr;
      int         row         = -1;
      if (auto* casted = dynamic_cast<const action_node*>(&n)) {
         const action_parent_node* parent = casted->parent;
         parent_void = parent;
         if (parent)
            row = parent->index_of_child(*casted);
      } else if (auto* casted = dynamic_cast<const idle_node*>(&n)) {
         const idle_parent_node* parent = casted->parent;
         parent_void = parent;
         if (parent)
            row = parent->index_of_child(*casted);
      } else if (auto* casted = dynamic_cast<const graph_node*>(&n)) {
         for (size_t i = 0; i < g_size; ++i) {
            if (graphs[i] == casted) {
               row = i;
               break;
            }
         }
      } else if (auto* casted = dynamic_cast<const idle_parent_node*>(&n)) {
         for (size_t i = 0; i < g_size; ++i) {
            if (graphs[i]->loose == casted) {
               row         = graphs[i]->children.size();
               parent_void = graphs[i];
               break;
            }
         }
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
         auto* loose_a = this->_datastore.loose.actions;
         auto* loose_i = this->_datastore.loose.idles;
         if (row == size) {
            if (loose_a && !loose_a->children.empty())
               return loose_a;
            if (loose_i && !loose_i->children.empty())
               return loose_i;
            return nullptr;
         }
         if (row == size + 1) {
            if (loose_a && !loose_a->children.empty())
               if (loose_i && !loose_i->children.empty())
                  return loose_i;
            return nullptr;
         }
         return nullptr;
      }
      if (auto* casted = dynamic_cast<const action_parent_node*>(parent)) {
         const auto&  list = casted->children;
         const size_t size = list.size();
         if (row < size)
            return list[row];
         //
         if (row == size) {
            if (auto* specific = dynamic_cast<const graph_node*>(casted)) {
               return specific->loose;
            }
         }
         return nullptr;
      }
      if (auto* casted = dynamic_cast<const idle_parent_node*>(parent)) {
         const auto&  list = casted->children;
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
   
   IdleAnimationFormsModel::action_node* IdleAnimationFormsModel::_node_for_action(graph_node* graph, const dovah::form_stub& stub) {
      if (graph)
         return _node_for_action(*graph, stub);
      return _node_for_loose_action(stub);
   }
   IdleAnimationFormsModel::action_node* IdleAnimationFormsModel::_node_for_action(graph_node& graph, const dovah::form_stub& stub) {
      for (action_node* node : graph.children)
         if (&node->stub == &stub)
            return node;
      return nullptr;
   }
   IdleAnimationFormsModel::action_node* IdleAnimationFormsModel::_node_for_loose_action(const dovah::form_stub& stub) {
      return this->_datastore.loose_action(stub);
   }
#pragma endregion

#pragma region Constraints
   bool IdleAnimationFormsModel::_can_create_new_idle_in(const idle_parent_node& parent) const {
      if (dynamic_cast<const action_node*>(&parent)) {
         return parent.children.empty();
      }
      return true;
   }
   bool IdleAnimationFormsModel::_can_ever_duplicate(const idle_node& n) const {
      if (!n.parent)
         return false;
      if (dynamic_cast<const action_node*>((const idle_parent_node*)n.parent))
         return false;
      return true;
   }
#pragma endregion

/*static*/ dovah::file_load_order* IdleAnimationFormsModel::_get_file_load_order() {
   auto files = DovahKitCore::get().get_loaded_files();
   if (files.empty())
      return nullptr;
   auto* file = files[0];
   if (!file)
      return nullptr;
   return &file->get_load_order();
}

#pragma region Form events
   void IdleAnimationFormsModel::_on_game_data_acquired() {
      this->_rebuild_datastore();
   }
   void IdleAnimationFormsModel::_on_game_data_abandon() {
      this->beginResetModel();
      this->_cache.clear();
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
            this->_recache_idle(stub);
            break;
      }
   }
   void IdleAnimationFormsModel::_on_form_deletion_imminent(dovah::form_stub& stub, bool just_being_flagged) {
      if (!just_being_flagged) {
         this->_datastore.on_before_form_deleted(stub);
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

   auto* flo = _get_file_load_order();
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
         for (idle_node* child : idle.children) {
            recurse(*child);
         }
      };
      auto _recache_action_tree = [this, &_recache_idle_tree](action_node& action) -> void {
         this->_recache_action(action);
         for (idle_node* idle : action.children)
            _recache_idle_tree(*idle);
      };

      for (auto* graph : this->_datastore.graphs) {
         auto& cached_graph = this->_cache[graph];
         cached_graph.display_string = QString::fromStdString(graph->path);
         for (action_node* action : graph->children)
            _recache_action_tree(*action);
         for (idle_node* idle : graph->loose->children)
            _recache_idle_tree(*idle);
      }
      for (action_node* action : this->_datastore.loose.actions->children)
         _recache_action_tree(*action);
      for (idle_node* idle : this->_datastore.loose.idles->children)
         _recache_idle_tree(*idle);
   }
   {
      auto& list = this->_datastore.warnings;
      if (!list.empty()) {
         auto& logger = dovahkit::subsystems::message_log::core::get();
         for (const auto* warning : list) {
            QString text;
            if (auto* casted = dynamic_cast<const datastore_warnings::cyclical_parent_relationships*>(warning)) {
               text = tr("Idle %1's parent chain forms a cyclical reference.")
                  .arg(editor_helpers::form_identifiers_to_string(&casted->idle.stub));
            } else if (auto* casted = dynamic_cast<const datastore_warnings::cyclical_sibling_relationships*>(warning)) {
               text = tr("Idle %1's previous-sibling chain forms a cyclical reference.")
                  .arg(editor_helpers::form_identifiers_to_string(&casted->idle.stub));
            } else if (auto* casted = dynamic_cast<const datastore_warnings::idle_has_multiple_next_siblings*>(warning)) {
               text = tr(
                  "Multiple idles are fighting to have %1 as their previous sibling. This can "
                  "happen if the idle tree has been overridden improperly by a mod."
               )
                  .arg(editor_helpers::form_identifiers_to_string(&casted->idle.stub));
            } else if (auto* casted = dynamic_cast<const datastore_warnings::inconsistent_parentage*>(warning)) {
               text = tr("Somehow, idle %1 is not present in its parent's child list.")
                  .arg(editor_helpers::form_identifiers_to_string(&casted->idle.stub));
            } else if (auto* casted = dynamic_cast<const datastore_warnings::orphaned_idle*>(warning)) {
               text = tr("Idle %1 is orphaned: it has no parent idle, and isn't an action root.")
                  .arg(editor_helpers::form_identifiers_to_string(&casted->idle.stub));
            } else if (auto* casted = dynamic_cast<const datastore_warnings::previous_sibling_is_not_as_expected*>(warning)) {
               if (auto* node = casted->previous_sibling.expected) {
                  text = tr("After the idle tree was fully built, idle %1 expected to be the next sibling of idle %2.");
                  text = text.arg(editor_helpers::form_identifiers_to_string(&casted->idle.stub));
                  text = text.arg(editor_helpers::form_identifiers_to_string(&node->stub));
               } else {
                  text = tr("After the idle tree was fully built, idle %1 expected to be the first (or possibly only) child of its parent.");
                  text = text.arg(editor_helpers::form_identifiers_to_string(&casted->idle.stub));
               }

               QString instead;
               if (auto* node = casted->previous_sibling.actual) {
                  instead = tr("Instead, the idle is located after %1.");
                  instead = instead.arg(editor_helpers::form_identifiers_to_string(&node->stub));
               } else {
                  instead = tr("Instead, the idle is its parent's first or only child.");
               }

               text = tr("%1 %2", "datastore warning sentence order for 'previous sibling is not as expected'").arg(text).arg(instead);
            } else if (auto* casted = dynamic_cast<const datastore_warnings::sibling_is_an_ancestor*>(warning)) {
               text = tr("Idle %1 has a previous sibling that is one of its ancestor idles.")
                  .arg(editor_helpers::form_identifiers_to_string(&casted->idle.stub));
            } else if (auto* casted = dynamic_cast<const datastore_warnings::siblings_have_mismatched_parents*>(warning)) {
               text = tr("Idle %1 has a different parent from one of its previous siblings.")
                  .arg(editor_helpers::form_identifiers_to_string(&casted->idle.stub));
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
      auto index = graph->index_of_child(stub);
      if (index == datastore_node::no_index)
         continue;
      this->_recache_action(*graph->children[index]);
      auto tl = _qmi_for_child_node(index, 0, *graph);
      auto br = _qmi_for_child_node(index, this->columnCount({}), *graph);
      emit dataChanged(tl, br);
   }
   {
      auto* parent = this->_datastore.loose.actions;
      auto  index = parent->index_of_child(stub);
      if (index != datastore_node::no_index) {
         this->_recache_action(*parent->children[index]);
         auto tl = _qmi_for_child_node(index, 0, *parent);
         auto br = _qmi_for_child_node(index, this->columnCount({}), *parent);
         emit dataChanged(tl, br);
      }
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
      auto* a_node = graph.action_by_stub(action);
      if (a_node)
         return *a_node;
      
      auto qmi = _qmi_for_node(graph);
      this->beginInsertRows(qmi, graph.children.size(), graph.children.size());
      {
         auto a_node_ptr = std::make_unique<action_node>(this->_datastore, action);
         a_node = a_node_ptr.get();
         graph.append_child(std::move(a_node_ptr));
         this->_recache_action(*a_node);
      }
      this->endInsertRows();
      //
      this->layoutAboutToBeChanged({ qmi }, LayoutChangeHint::VerticalSortHint);
      auto mapping = graph.sort_children_and_remember();
      {  // Update QPMIs.
         //
         // QAbstractItemModel offers `changePersistentIndex` and `changePersistentIndexList`. 
         // The former may seem preferable because it doesn't require you to create two lists 
         // to throw away; just change the indices one at a time, right? The problem is that 
         // if you don't change all of the indices at once, using the list approach, then one 
         // index can be changed to become identical to another index that has yet to be changed, 
         // such that trying to change the latter index corrupts the former index.
         // 
         // If you need to change multiple QPMIs (which, in practice, is basically always), then 
         // the ONLY safe option is `changePersistentIndexList`. The only question, then, is 
         // this: do you loop over the collection of rows you've reordered and blindly change 
         // every possibly QPMI, or do you loop over only those QPMIs that already exist?
         //
         QModelIndexList list_prior;
         QModelIndexList list_after;
         //
         auto   all_qpmis   = this->persistentIndexList(); // implicitly shared, so no overhead here
         size_t blind_count = mapping.size() * ColumnCount;
         if (blind_count > all_qpmis.size()) {
            //
            // There are more cells to (potentially) update than there are extant QPMIs, so 
            // just loop over the latter instead.
            //
            for (const auto& qpmi : all_qpmis) {
               if (!_qmi_is_child_of(qpmi, graph)) // ignore irrelevant QPMIs
                  continue;

               int row = qpmi.row();
               if (row == mapping.size()) { // special-case for graphs' "loose" nodes
                  //
                  // A graph's "loose" node exists after its child actions, and so outside of the 
                  // `mapping` variable. We can leave the row unchanged.
                  //
                  continue;
               }
               if (row < 0 || row >= mapping.size())
                  row = -1;
               else {
                  for (size_t i = 0; i < mapping.size(); ++i) {
                     if (mapping[i] == row) {
                        row = i;
                        break;
                     }
                  }
               }

               list_prior.push_back(qpmi);
               list_after.push_back(this->createIndex(row, qpmi.column(), qpmi.internalPointer()));
            }
         } else {
            //
            // There are fewer cells to (potentially) update than there are extant QPMIs, so 
            // loop over the former. We can freely update a QPMI that doesn't actually exist 
            // (i.e. a cell that isn't referred to by any extant QPMI) without causing any 
            // problems.
            //
            list_prior.clear();
            for (size_t index_after = 0; index_after < mapping.size(); ++index_after) {
               size_t index_prior = mapping[index_after];
               if (index_prior == index_after)
                  continue;

               for (size_t col = 0; col < ColumnCount; ++col) {
                  QModelIndex qmi_prior = this->createIndex(index_prior, col, &graph);
                  QModelIndex qmi_after = this->createIndex(index_after, col, &graph);
                  list_prior.push_back(qmi_prior);
                  list_after.push_back(qmi_after);
               }
            }
         }
         this->changePersistentIndexList(list_prior, list_after);
      }
      this->layoutChanged({ qmi }, LayoutChangeHint::VerticalSortHint);
      //
      return *a_node;
   }
   IdleAnimationFormsModel::idle_node* IdleAnimationFormsModel::_create_action_root(graph_node& graph, dovah::form_stub& action, QString idle_editor_id) {
      dovah::form_stub* idle_stub = this->_try_silently_create_idle(idle_editor_id);
      if (!idle_stub)
         return nullptr;

      auto& a_node = _get_or_create_action(graph, action);
      auto  i_node_ptr = std::make_unique<idle_node>(this->_datastore, *idle_stub);
      auto* i_node     = i_node_ptr.get();
      assert(i_node != nullptr);
      this->_datastore.append_idle_in(*i_node, a_node);
      i_node_ptr.release();
      return i_node;
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
      auto* graph = this->_datastore.get_or_create_graph_by_path(path.toStdString());
      if (!graph)
         return {};

      auto& cached_graph = this->_cache[graph];
      cached_graph.display_string = QString::fromStdString(graph->path);

      return _qmi_for_node(*graph);
   }

   [[nodiscard]] std::vector<dovah::form_stub*> IdleAnimationFormsModel::_actionsByGraph(const graph_node& graph) const noexcept {
      std::vector<dovah::form_stub*> stubs;
      for (const action_node* action : graph.children) {
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
      auto* casted = dynamic_cast<const idle_parent_node*>(node);
      if (!casted)
         return false;
      return _can_create_new_idle_in(*casted);
   }
   QModelIndex IdleAnimationFormsModel::createIdle(const QModelIndex& parent_qmi, QString idle_editor_id) {
      idle_parent_node* parent_node = nullptr;
      {
         auto* node = _node_for_qmi(parent_qmi);
         if (!node)
            return {};
         parent_node = dynamic_cast<idle_parent_node*>(node);
         if (!parent_node)
            return {};
      }
      if (!_can_create_new_idle_in(*parent_node))
         return {};

      dovah::form_stub* stub = this->_try_silently_create_idle(idle_editor_id);
      if (!stub)
         return {};

      auto  i_node_ptr = std::make_unique<idle_node>(this->_datastore, *stub);
      auto* i_node     = i_node_ptr.get();
      assert(i_node != nullptr);
      this->_datastore.append_idle_in(*i_node, *parent_node);
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
            auto* flo = _get_file_load_order();
            if (!flo)
               return {};

            size_t count_to_duplicate = [](this auto&& recurse, idle_node& idle) -> size_t {
               size_t count = 1;
               for (auto& child_ptr : idle.children) {
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
         assert(src_idle->parent);
         QModelIndex root_qmi = {};
         [this, &root_qmi](this auto&& recurse, idle_node& idle, idle_parent_node& dst_parent, bool is_root = false) -> void {
            dovah::form_stub* stub = this->_try_silently_duplicate_idle(idle.stub);
            if (!stub)
               return;

            auto  i_node_ptr = std::make_unique<idle_node>(this->_datastore, *stub);
            auto* i_node = i_node_ptr.get();
            assert(i_node != nullptr);
            if (is_root) {
               this->_datastore.place_idle_after(*i_node, idle);
            } else {
               this->_datastore.append_idle_in(*i_node, dst_parent);
            }
            i_node_ptr.release();
            if (is_root) {
               root_qmi = _qmi_for_node(*i_node);
            }

            for (auto& child_ptr : idle.children) {
               recurse(*child_ptr, *i_node);
            }
         }(*src_idle, *src_idle->parent, true);
         return root_qmi;
      } else {
         dovah::form_stub* stub = this->_try_silently_duplicate_idle(src_idle->stub);
         if (!stub)
            return {};

         auto  i_node_ptr = std::make_unique<idle_node>(this->_datastore, *stub);
         auto* i_node = i_node_ptr.get();
         assert(i_node != nullptr);
         this->_datastore.place_idle_after(*i_node, *src_idle);
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

      // Don't allow deleting action roots unless they're from the active file. The way the 
      // CK and game parse IDLE/ANAM suggests to me that deleting action roots via an override 
      // may not be fully reliable.
      if (dynamic_cast<const action_node*>((const idle_parent_node*)src_idle->parent)) {
         auto& stub = src_idle->stub;
         auto& lo   = stub.get_owning_load_order();
         if (!lo.is_defined_in_active_file(stub))
            return false;
      }

      return true;
   }
   void IdleAnimationFormsModel::deleteIdle(const QModelIndex& idle_qmi, QWidget* error_dialog_parent) {
      auto* src_node = _node_for_qmi(idle_qmi);
      if (!src_node)
         return;
      auto* src_idle = dynamic_cast<const idle_node*>(src_node);
      if (!src_idle) // QMI is not that of an idle
         return;

      std::vector<dovah::form_stub*> stubs_to_delete;
      [&stubs_to_delete](this auto&& recurse, const idle_node& node) -> void {
         stubs_to_delete.push_back(&node.stub);
         for (auto& child_ptr : node.children) {
            recurse(*child_ptr);
         }
      }(*src_idle);

      auto& editor = DovahKitCore::get();
      bool any_failed = false;
      for (auto* stub : stubs_to_delete) {
         editor.delete_form(
            *stub,
            [](const dovah::form_deletion_request&) {
               return true;
            },
            [&any_failed](const dovah::exceptions::form_deletion_failed& ex) {
               any_failed = true;
            },
            [](const dovah::form_deletion_request& request) {}
         );
      }
      if (any_failed) {
         QMessageBox::critical(
            error_dialog_parent,
            QObject::tr("Error", "delete form error"),
            QObject::tr("Unable to delete all of the needed idles.")
         );
      }
   }
#pragma endregion

#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex IdleAnimationFormsModel::index(int row, int column, const QModelIndex& parent_qmi) const /*override*/ {
         if (row < 0 || column < 0)
            return {};
         const datastore_node* parent_node = _node_for_qmi(parent_qmi);
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
            parent = casted->parent;
         } else if (auto* casted = dynamic_cast<const idle_node*>(subject)) {
            parent = casted->parent;
         } else if (auto* casted = dynamic_cast<const idle_parent_node*>(subject)) {
            for (auto* graph : this->_datastore.graphs) {
               if (graph->loose == casted) {
                  parent = graph;
                  break;
               }
            }
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
            if (!casted->parent)
               return {};
            if (!_child_node_by_row(casted->parent, row))
               return {};
            return _qmi_for_child_node(row, column, *casted->parent);
         } else if (auto* casted = dynamic_cast<const idle_node*>(basis)) {
            if (!casted->parent)
               return {};
            if (row >= casted->children.size())
               return {};
            return _qmi_for_child_node(row, column, *casted->parent);
         } else if (dynamic_cast<const graph_node*>(basis)) {
            basis_is_top_level = true;
         } else if (auto* casted = dynamic_cast<const idle_parent_node*>(basis)) {
            basis_is_top_level = true;
            for (auto* graph : this->_datastore.graphs) {
               if (graph->loose == casted) {
                  return _qmi_for_child_node(row, column, *graph);
               }
            }
         } else if (auto* casted = dynamic_cast<const action_parent_node*>(basis)) {
            basis_is_top_level = true;
         }
         if (basis_is_top_level) {
            const size_t size = this->_datastore.graphs.size();
            if (row < size)
               return _qmi_for_node(*this->_datastore.graphs[row], column);
            datastore_node* subject = nullptr;
            if (row == size) {
               auto* loose_a = this->_datastore.loose.actions;
               auto* loose_i = this->_datastore.loose.idles;
               if (loose_a && !loose_a->children.empty())
                  subject = loose_a;
               else if (loose_i && !loose_i->children.empty())
                  subject = loose_i;
            } else if (row == size + 1) {
               auto* loose_a = this->_datastore.loose.actions;
               auto* loose_i = this->_datastore.loose.idles;
               if (loose_a && !loose_a->children.empty())
                  if (loose_i && !loose_i->children.empty())
                     subject = loose_i;
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
            const auto* loose_a = this->_datastore.loose.actions;
            const auto* loose_i = this->_datastore.loose.idles;
            if (loose_a && !loose_a->children.empty())
               ++size;
            if (loose_i && !loose_i->children.empty())
               ++size;
            return size;
         }
         size_t size = 0;
         if (auto* casted = dynamic_cast<const action_parent_node*>(node)) {
            size = casted->children.size();
            if (auto* subtype = dynamic_cast<const graph_node*>(casted))
               if (subtype->loose)
                  ++size;
         } else if (auto* casted = dynamic_cast<const idle_parent_node*>(node)) {
            size = casted->children.size();
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
               if (dynamic_cast<const action_parent_node*>(node))
                  return QVariant::fromValue(NodeType::LooseActionsPerModel);
               if (auto* loose = dynamic_cast<const idle_parent_node*>(node)) {
                  if (loose == this->_datastore.loose.idles)
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
               if (node == this->_datastore.loose.actions)
                  return tr("LOOSE ACTIONS");
               if (node == this->_datastore.loose.idles)
                  return tr("LOOSE");
               if (auto* casted = dynamic_cast<const idle_parent_node*>(node)) {
                  for (auto* graph : this->_datastore.graphs) {
                     if (casted == graph->loose)
                        return tr("LOOSE");
                  }
               }
               break;
            case Qt::ForegroundRole:
               if (dynamic_cast<const action_node*>(node)) {
                  return QColor(64, 160, 255);
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
         flags |= Qt::ItemIsDragEnabled;
         if (auto* casted = dynamic_cast<const action_node*>(node)) {
            //
            // Try to limit actions to just one child node (though the data structure allows 
            // multiple, to account for edge-cases involving multiple mods applying a new root 
            // idle to the same action on the same behavior graph).
            //
            if (casted->children.empty()) {
               flags |= Qt::ItemIsDropEnabled;
            }
         } else {
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
#if 0
      /*virtual*/ QMimeData* IdleAnimationFormsModel::mimeData(const QModelIndexList& indices) const /*override*/;
      /*virtual*/ bool IdleAnimationFormsModel::canDropMimeData(const QMimeData* mime, Qt::DropAction action, int row, int column, const QModelIndex& parent) const /*override*/;
      /*virtual*/ bool IdleAnimationFormsModel::dropMimeData(const QMimeData* mime, Qt::DropAction action, int row, int column, const QModelIndex& parent) /*override*/;
#endif
   #pragma endregion
#pragma endregion