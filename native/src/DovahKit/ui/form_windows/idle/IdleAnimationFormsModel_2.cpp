#include "./IdleAnimationFormsModel_2.h"
#include <QColor>
#include "dovah/datastores/idles/action_node.h"
#include "dovah/datastores/idles/graph_node.h"
#include "dovah/datastores/idles/idle_node.h"
#include "dovah/forms/IdleAnimation.h"
#include "dovah/form_stub.h"
#include "dovahscript/dovahscript_host.h"
#include "editor/core.h"
#include "dovah/files/tes_file_reading/file_loader.h"

namespace {
   constexpr const char* const mime_type = "application/dovah-kit.idle-animation-forms-model.node";
}

IdleAnimationFormsModel_2::IdleAnimationFormsModel_2(QObject* parent) : QAbstractItemModel(parent) {
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
            auto& cb_set = this->_datastore.callbacks.graphs.on_inserted;
            cb_set.before = [this](size_t index) {
               this->beginInsertRows(_qmi_for_model_root(), index, index);
            };
            cb_set.after = [this](const graph_node&) {
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
            cb_set.after = [this]() {
               if (this->_callback_state.emitted_last_deletion)
                  this->endRemoveRows();
               this->_callback_state.emitted_last_deletion = false;
            };
         }
         {
            auto& cb_set = this->_datastore.callbacks.actions.on_inserted;
            cb_set.before = [this](const action_parent_node& parent, size_t index) {
               this->beginInsertRows(_qmi_for_node(parent), index, index);
            };
            cb_set.after = [this](const action_node&) {
               this->endInsertRows();
            };
         }
         {
            auto& cb_set = this->_datastore.callbacks.actions.on_moved;
            cb_set.before = [this](const action_parent_node& parent_prior, size_t from, const action_parent_node& parent_after, size_t to) {
               auto qmi_prior = _qmi_for_node(parent_prior);
               auto qmi_after = _qmi_for_node(parent_after);
               if (to >= from)
                  ++to;
               this->beginMoveRows(qmi_prior, from, from, qmi_after, to);
            };
            cb_set.after = [this]() {
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
            cb_set.after = [this]() {
               if (this->_callback_state.emitted_last_deletion)
                  this->endRemoveRows();
               this->_callback_state.emitted_last_deletion = false;
            };
         }
         {
            auto& cb_set = this->_datastore.callbacks.idles.on_inserted;
            cb_set.before = [this](const idle_parent_node& parent, size_t index) {
               this->beginInsertRows(_qmi_for_node(parent), index, index);
            };
            cb_set.after = [this](const idle_node&) {
               this->endInsertRows();
            };
         }
         {
            auto& cb_set = this->_datastore.callbacks.idles.on_moved;
            cb_set.before = [this](const idle_parent_node& parent_prior, size_t from, const idle_parent_node& parent_after, size_t to) {
               auto qmi_prior = _qmi_for_node(parent_prior);
               auto qmi_after = _qmi_for_node(parent_after);
               if (to >= from)
                  ++to;
               this->beginMoveRows(qmi_prior, from, from, qmi_after, to);
            };
            cb_set.after = [this]() {
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
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent,  this, &IdleAnimationFormsModel_2::_on_game_data_abandon);
   QObject::connect(&editor, &DovahKitCore::dataAcquireComplete,  this, &IdleAnimationFormsModel_2::_on_game_data_acquired);
   QObject::connect(&editor, &DovahKitCore::formCreated,          this, [this](dovah::form_stub* stub) { this->_on_form_created(*stub); });
   QObject::connect(&editor, &DovahKitCore::formModified,         this, [this](dovah::form_stub* stub) { this->_on_form_modified(*stub); });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub, bool flag) { this->_on_form_deletion_imminent(*stub, flag); });
   if (editor.has_data()) {
      this->_rebuild_datastore();
   }
}

#pragma region Node utils
   QModelIndex IdleAnimationFormsModel_2::_qmi_for_model_root() const {
      return {};
   }
   QModelIndex IdleAnimationFormsModel_2::_qmi_for_node(const datastore_node& n, int column) const {
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
   QModelIndex IdleAnimationFormsModel_2::_qmi_for_child_node(int row, int column, const datastore_node& parent) const {
      return this->createIndex(row, column, (quintptr)&parent);
   }

   const IdleAnimationFormsModel_2::datastore_node* IdleAnimationFormsModel_2::_child_node_by_row(const datastore_node* parent, size_t row) const {
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

   const IdleAnimationFormsModel_2::datastore_node* IdleAnimationFormsModel_2::_node_for_qmi(const QModelIndex& qmi) const {
      if (!qmi.isValid())
         return nullptr;
      auto* parent = (datastore_node*)qmi.internalPointer();
      return _child_node_by_row(parent, qmi.row());
   }
   IdleAnimationFormsModel_2::datastore_node* IdleAnimationFormsModel_2::_node_for_qmi(const QModelIndex& qmi) {
      return const_cast<datastore_node*>(std::as_const(*this)._node_for_qmi(qmi));
   }

   const IdleAnimationFormsModel_2::datastore_node* IdleAnimationFormsModel_2::_child_node_for_qmi(const QModelIndex& parent_qmi, int row) const {
      if (parent_qmi == _qmi_for_model_root()) {
         return _child_node_by_row(nullptr, row);
      }
      const auto* parent_node = _node_for_qmi(parent_qmi);
      if (!parent_node)
         return nullptr;
      return _child_node_by_row(parent_node, row);
   }
   IdleAnimationFormsModel_2::datastore_node* IdleAnimationFormsModel_2::_child_node_for_qmi(const QModelIndex& parent_qmi, int row) {
      return const_cast<datastore_node*>(std::as_const(*this)._child_node_for_qmi(parent_qmi, row));
   }

   const IdleAnimationFormsModel_2::graph_node* IdleAnimationFormsModel_2::_node_for_graph_path(QString path) const {
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
   IdleAnimationFormsModel_2::graph_node* IdleAnimationFormsModel_2::_node_for_graph_path(QString path) {
      return const_cast<graph_node*>(std::as_const(*this)._node_for_graph_path(path));
   }
   
   IdleAnimationFormsModel_2::action_node* IdleAnimationFormsModel_2::_node_for_action(graph_node* graph, const dovah::form_stub& stub) {
      if (graph)
         return _node_for_action(*graph, stub);
      return _node_for_loose_action(stub);
   }
   IdleAnimationFormsModel_2::action_node* IdleAnimationFormsModel_2::_node_for_action(graph_node& graph, const dovah::form_stub& stub) {
      for (action_node* node : graph.children)
         if (&node->stub == &stub)
            return node;
      return nullptr;
   }
   IdleAnimationFormsModel_2::action_node* IdleAnimationFormsModel_2::_node_for_loose_action(const dovah::form_stub& stub) {
      return this->_datastore.loose_action(stub);
   }
#pragma endregion

#pragma region Form events
   void IdleAnimationFormsModel_2::_on_game_data_acquired() {
      this->_rebuild_datastore();
   }
   void IdleAnimationFormsModel_2::_on_game_data_abandon() {
      this->beginResetModel();
      this->_cache.clear();
      this->_datastore.reset();
      this->endResetModel();
   }
   void IdleAnimationFormsModel_2::_on_form_created(dovah::form_stub& stub) {
      if (stub.form_type != dovah::form_type::idle)
         return;
      this->_datastore.on_idle_created(stub);
   }
   void IdleAnimationFormsModel_2::_on_form_modified(dovah::form_stub& stub) {
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
   void IdleAnimationFormsModel_2::_on_form_deletion_imminent(dovah::form_stub& stub, bool just_being_flagged) {
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
#pragma endregion

void IdleAnimationFormsModel_2::_rebuild_datastore() {
   this->_cache.clear();

   auto& editor = DovahKitCore::get();
   //
   // Disgusting hack to access the file_load_order:
   //
   auto files = editor.get_loaded_files();
   if (files.empty()) {
      this->beginResetModel();
      this->_datastore.reset();
      this->endResetModel();
      return;
   }
   auto* file = files[0];
   if (!file) {
      this->beginResetModel();
      this->_datastore.reset();
      this->endResetModel();
      return;
   }
   //
   this->beginResetModel();
   this->_datastore.build(file->get_load_order());
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
   this->endResetModel();
}
void IdleAnimationFormsModel_2::_recache_action(const action_node& node) {
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
void IdleAnimationFormsModel_2::_recache_action(const dovah::form_stub& stub) {
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
void IdleAnimationFormsModel_2::_recache_idle(const idle_node& node) {
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
void IdleAnimationFormsModel_2::_recache_idle(const dovah::form_stub& stub) {
   auto* node = this->_datastore.idle_by_stub(stub);
   if (node) {
      this->_recache_idle(*node);
      auto tl = _qmi_for_node(*node);
      auto br = _qmi_for_node(*node, this->columnCount({}) - 1);
      emit dataChanged(tl, br);
   }
}

#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex IdleAnimationFormsModel_2::index(int row, int column, const QModelIndex& parent_qmi) const /*override*/ {
         if (row < 0 || column < 0)
            return {};
         const datastore_node* parent_node = _node_for_qmi(parent_qmi);
         const datastore_node* target_node = _child_node_by_row(parent_node, row);
         if (!target_node)
            return {};
         return _qmi_for_node(*target_node, column);
      }
      /*virtual*/ QModelIndex IdleAnimationFormsModel_2::parent(const QModelIndex& index) const /*override*/ {
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
      /*virtual*/ QModelIndex IdleAnimationFormsModel_2::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
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
      /*virtual*/ int IdleAnimationFormsModel_2::rowCount(const QModelIndex& parent) const /*override*/ {
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
      /*virtual*/ int IdleAnimationFormsModel_2::columnCount(const QModelIndex& parent) const /*override*/ {
         return 1;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant IdleAnimationFormsModel_2::data(const QModelIndex& qmi, int role) const /*override*/ {
         const datastore_node* node = _node_for_qmi(qmi);
         if (!node)
            return {};
         switch (role) {
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
      /*virtual*/ Qt::ItemFlags IdleAnimationFormsModel_2::flags(const QModelIndex& index) const /*override*/ {
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
   /*virtual*/ QVariant IdleAnimationFormsModel_2::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
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
         /*virtual*/ QStringList IdleAnimationFormsModel_2::mimeTypes() const /*override*/ {
            return { QString::fromLatin1(mime_type) };
         }
         /*virtual*/ Qt::DropActions IdleAnimationFormsModel_2::supportedDropActions() const /*override*/ {
            return Qt::DropAction::CopyAction | Qt::DropAction::MoveAction;
         }
      #pragma endregion
#if 0
      /*virtual*/ QMimeData* IdleAnimationFormsModel_2::mimeData(const QModelIndexList& indices) const /*override*/;
      /*virtual*/ bool IdleAnimationFormsModel_2::canDropMimeData(const QMimeData* mime, Qt::DropAction action, int row, int column, const QModelIndex& parent) const /*override*/;
      /*virtual*/ bool IdleAnimationFormsModel_2::dropMimeData(const QMimeData* mime, Qt::DropAction action, int row, int column, const QModelIndex& parent) /*override*/;
#endif
   #pragma endregion
#pragma endregion