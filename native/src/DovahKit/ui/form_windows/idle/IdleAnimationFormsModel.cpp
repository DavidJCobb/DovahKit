#include "./IdleAnimationFormsModel.h"
#include <QColor>
#include "./IdleAnimationFormsModel_impl/action_node.h"
#include "./IdleAnimationFormsModel_impl/idle_node.h"
#include "./IdleAnimationFormsModel_impl/_unique_ptr_utils.h"
#include "dovah/forms/IdleAnimation.h"
#include "dovah/form_stub.h"
#include "editor/core.h"

namespace {
   namespace node_utils {
      using namespace IdleAnimationFormsModel_impl::utils;
   }
   using IdleAnimationFormsModel_impl::make_unique;

   constexpr const char* const mime_type = "application/dovah-kit.idle-animation-forms-model.node";
}

IdleAnimationFormsModel::IdleAnimationFormsModel(QObject* parent) : QAbstractItemModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent,  this, &IdleAnimationFormsModel::_on_game_data_abandon);
   QObject::connect(&editor, &DovahKitCore::dataAcquireComplete,  this, &IdleAnimationFormsModel::_on_game_data_acquired);
   QObject::connect(&editor, &DovahKitCore::formCreated,          this, [this](dovah::form_stub* stub) { this->_on_form_created(*stub); });
   QObject::connect(&editor, &DovahKitCore::formModified,         this, [this](dovah::form_stub* stub) { this->_on_form_modified(*stub); });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub, bool flag) { this->_on_form_deletion_imminent(*stub, flag); });
}

#pragma region Node utils
   QModelIndex IdleAnimationFormsModel::_qmi_for_model_root() const {
      return {};
   }
   QModelIndex IdleAnimationFormsModel::_qmi_for_node(const node& n, int column) const {
      if (column < 0)
         return {};
      const auto& graphs = this->nodes.graphs;
      const auto  g_size = graphs.size();

      if (&n == this->nodes.loose.actions.get()) {
         return this->createIndex(g_size, column, nullptr);
      } else if (&n == this->nodes.loose.idles.get()) {
         return this->createIndex(g_size + 1, column, nullptr);
      }

      const void* parent_void = nullptr;
      int         row         = -1;
      if (auto* casted = dynamic_cast<const action_node*>(&n)) {
         const auto* parent = casted->parent;
         parent_void = parent;
         if (parent)
            row = parent->index_of_child(*casted);
      } else if (auto* casted = dynamic_cast<const idle_node*>(&n)) {
         const auto* parent = casted->parent;
         parent_void = parent;
         if (parent)
            row = parent->index_of_child(*casted);
      } else if (auto* casted = dynamic_cast<const graph_node*>(&n)) {
         for (size_t i = 0; i < g_size; ++i) {
            if (graphs[i].get() == casted) {
               row = i;
               break;
            }
         }
      } else if (auto* casted = dynamic_cast<const loose_idle_parent_node*>(&n)) {
         if (casted->owner) {
            auto* owner_graph = dynamic_cast<const graph_node*>(casted->owner);
            assert(!!owner_graph);
            row         = owner_graph->children.size();
            parent_void = owner_graph;
         }
      }
      if (row == -1)
         return {};
      return this->createIndex(row, column, (quintptr)parent_void);
   }
   QModelIndex IdleAnimationFormsModel::_qmi_for_child_node(int row, int column, const node& parent) const {
      return this->createIndex(row, column, (quintptr)&parent);
   }

   const IdleAnimationFormsModel::node* IdleAnimationFormsModel::_child_node_by_row(const node* parent, size_t row) const {
      if (!parent) {
         const auto&  list = this->nodes.graphs;
         const size_t size = list.size();
         if (row < size)
            return list[row].get();
         if (row == size) {
            node* subject = this->nodes.loose.actions.get();
            if (!subject)
               subject = this->nodes.loose.idles.get();
            return subject;
         }
         if (row == size + 1) {
            node* subject = this->nodes.loose.idles.get();
            if (this->nodes.loose.actions && subject)
               return subject;
         }
         return nullptr;
      }
      if (auto* casted = dynamic_cast<const action_parent_node*>(parent)) {
         const auto&  list = casted->children;
         const size_t size = list.size();
         if (row < size)
            return list[row].get();
         //
         if (auto* specific = dynamic_cast<const graph_node*>(casted)) {
            if (row == size)
               return specific->loose.idles.get();
         }
         return nullptr;
      }
      if (auto* casted = dynamic_cast<const idle_parent_node*>(parent)) {
         const auto&  list = casted->children;
         const size_t size = list.size();
         if (row < size)
            return list[row].get();
         return nullptr;
      }
      return nullptr;
   }

   const IdleAnimationFormsModel::node* IdleAnimationFormsModel::_node_for_qmi(const QModelIndex& qmi) const {
      if (!qmi.isValid())
         return nullptr;
      auto* parent = (node*)qmi.internalPointer();
      return _child_node_by_row(parent, qmi.row());
   }
   IdleAnimationFormsModel::node* IdleAnimationFormsModel::_node_for_qmi(const QModelIndex& qmi) {
      return const_cast<node*>(std::as_const(*this)._node_for_qmi(qmi));
   }

   const IdleAnimationFormsModel::node* IdleAnimationFormsModel::_child_node_for_qmi(const QModelIndex& parent_qmi, int row) const {
      if (parent_qmi == _qmi_for_model_root()) {
         return _child_node_by_row(nullptr, row);
      }
      const auto* parent_node = _node_for_qmi(parent_qmi);
      if (!parent_node)
         return nullptr;
      return _child_node_by_row(parent_node, row);
   }
   IdleAnimationFormsModel::node* IdleAnimationFormsModel::_child_node_for_qmi(const QModelIndex& parent_qmi, int row) {
      return const_cast<node*>(std::as_const(*this)._child_node_for_qmi(parent_qmi, row));
   }

   const IdleAnimationFormsModel::graph_node* IdleAnimationFormsModel::_node_for_graph_path(QString path) const {
      for (auto& node_ptr : this->nodes.graphs) {
         if (node_ptr->path.compare(path, Qt::CaseInsensitive) == 0)
            return node_ptr.get();
      }
      return nullptr;
   }
   IdleAnimationFormsModel::graph_node* IdleAnimationFormsModel::_node_for_graph_path(QString path) {
      return const_cast<graph_node*>(std::as_const(*this)._node_for_graph_path(path));
   }
   
   IdleAnimationFormsModel::action_node* IdleAnimationFormsModel::_node_for_action(const graph_node* graph, const dovah::form_stub& stub) {
      if (graph)
         return _node_for_action(*graph, stub);
      return _node_for_loose_action(stub);
   }
   IdleAnimationFormsModel::action_node* IdleAnimationFormsModel::_node_for_action(const graph_node& graph, const dovah::form_stub& stub) {
      for (auto& node_ptr : graph.children)
         if (node_ptr->stub == &stub)
            return node_ptr.get();
      return nullptr;
   }
   IdleAnimationFormsModel::action_node* IdleAnimationFormsModel::_node_for_loose_action(const dovah::form_stub& stub) {
      auto& loose_container_ptr = this->nodes.loose.actions;
      if (!loose_container_ptr)
         return nullptr;
      for (auto& node_ptr : loose_container_ptr->children)
         if (node_ptr->stub == &stub)
            return node_ptr.get();
      return nullptr;
   }
#pragma endregion

void IdleAnimationFormsModel::_update_idle_node_parent(idle_node& node) {
   auto loaded = node.stub->load().ptr_cast<dovah::loaded_forms::IdleAnimation>();
   assert(!!loaded);
   if (auto* casted = node.parent->as<node_type::action>()) {
      loaded->parent.set(*loaded, casted->stub);
   } else if (auto* casted = node.parent->as<node_type::idle>()) {
      loaded->parent.set(*loaded, casted->stub);
   } else {
      loaded->parent.set(*loaded, nullptr);
   }
}

#pragma region Form events
   void IdleAnimationFormsModel::_on_game_data_acquired() {
      this->import_all_idles();
   }
   void IdleAnimationFormsModel::_on_game_data_abandon() {
      this->beginResetModel();
      this->idle_forms_to_nodes.clear();
      this->nodes = {};
      this->endResetModel();
   }
   void IdleAnimationFormsModel::_on_form_created(dovah::form_stub& stub) {
      if (stub.form_type != dovah::form_type::idle)
         return;
      this->import_idle(stub, true);
   }
   void IdleAnimationFormsModel::_on_form_modified(dovah::form_stub& stub) {
      switch (stub.form_type) {
         case dovah::form_type::action:
            {
               for (auto& graph_ptr : this->nodes.graphs) {
                  auto row = graph_ptr->index_of_form(stub);
                  if (row == action_parent_node::no_index)
                     continue;
                  auto* node = graph_ptr->children[row].get();
                  node->update_cached_form_data();
                  auto tl = _qmi_for_child_node(row, 0, *graph_ptr);
                  auto br = _qmi_for_child_node(row, this->columnCount({}), *graph_ptr);
                  emit dataChanged(tl, br);
               }
               if (auto& parent_ptr = this->nodes.loose.actions; parent_ptr) {
                  auto row = parent_ptr->index_of_form(stub);
                  if (row != action_parent_node::no_index) {
                     auto* node = parent_ptr->children[row].get();
                     node->update_cached_form_data();
                     auto tl = _qmi_for_child_node(row, 0, *parent_ptr);
                     auto br = _qmi_for_child_node(row, this->columnCount({}), *parent_ptr);
                     emit dataChanged(tl, br);
                  }
               }
            }
            break;
         case dovah::form_type::idle:
            {
               auto& map = this->idle_forms_to_nodes;
               auto  it  = map.find(&stub);
               if (it != map.end())
                  return;
               auto* node = it->second;
               node->update_cached_form_data();
               auto tl = _qmi_for_node(*node, 0);
               auto br = _qmi_for_node(*node, this->columnCount({}));
               emit dataChanged(tl, br);
               //
               // TODO: Check whether the IDLE's specified graph or parent has changed; if so, reparent 
               //       the node to match.
               //
            }
            break;
      }
   }
   void IdleAnimationFormsModel::_on_form_deletion_imminent(dovah::form_stub& stub, bool just_being_flagged) {
      switch (stub.form_type) {
         case dovah::form_type::action:
            {
               auto functor = [this, &stub, just_being_flagged](action_parent_node& parent, idle_parent_node& loose_node) {
                  auto&  action_list  = parent.children;
                  size_t action_count = action_list.size();
                  for (size_t i = 0; i < action_count; ++i) {
                     node_unique_ptr<action_node>& action_ptr = action_list[i];
                     if (action_ptr->stub != &stub)
                        continue;
                     this->_reparent_child_idles(*action_ptr, loose_node, _qmi_for_node(*action_ptr), _qmi_for_node(loose_node));
                     if (!just_being_flagged) {
                        //
                        // Delete the action node.
                        //
                        this->beginRemoveRows(_qmi_for_node(parent), i, i);
                        action_list.erase(action_list.begin() + i);
                        --i;
                        --action_count;
                        this->endRemoveRows();
                        continue;
                     }
                  }
               };
               functor(*this->nodes.loose.actions, *this->nodes.loose.idles);
               for (auto& graph_ptr : this->nodes.graphs) {
                  functor(*graph_ptr, *graph_ptr->loose.idles);
               }
            }
            break;
         case dovah::form_type::idle:
            {
               auto it = this->idle_forms_to_nodes.find(&stub);
               if (it == this->idle_forms_to_nodes.end())
                  return;
               if (!just_being_flagged) {
                  auto* idle_as_node = it->second;
                  auto* parent_node  = idle_as_node->parent;
                  this->idle_forms_to_nodes.erase(it);
                  assert(parent_node != nullptr);
                  //
                  // Reparent the idle's children before we destroy it.
                  //
                  if (auto* casted = dynamic_cast<idle_parent_node*>(parent_node)) {
                     auto& parent     = *casted;
                     auto  parent_qmi = _qmi_for_node(parent);
                     this->_reparent_child_idles(*idle_as_node, parent, _qmi_for_node(*idle_as_node), parent_qmi);
                     auto child_idx = parent.index_of_child(*idle_as_node);
                     if (child_idx != idle_parent_node::no_index) {
                        this->beginRemoveRows(parent_qmi, child_idx, child_idx);
                        parent.take_child(child_idx);
                        this->endRemoveRows();
                     }
                  }
               }
            }
            break;
      }
   }
#pragma endregion

IdleAnimationFormsModel::graph_node* IdleAnimationFormsModel::_get_or_emplace_graph(QString path, bool emit_signals_for_emplace) {
   auto* node = _node_for_graph_path(path);
   if (node)
      return node;

   size_t insert_at = this->nodes.graphs.size(); // TODO: sort alphabetically by path segments
   if (emit_signals_for_emplace) {
      this->beginInsertRows({}, insert_at, insert_at);
   }
   auto& node_ptr = this->nodes.graphs.emplace_back(make_unique<graph_node>());
   node_ptr->path = path;
   if (emit_signals_for_emplace) {
      this->endInsertRows();
   }
   return node_ptr.get();
}
void IdleAnimationFormsModel::_insert_idle_into(node_unique_ptr<idle_node>&& node_ptr, idle_parent_node& parent, bool emit_signals) {
   assert(!!node_ptr);
   assert(node_ptr->parent == nullptr);
   auto&  list      = parent.children;
   size_t insert_at = list.size(); // TODO: sort alphabetically by path segments
   if (emit_signals) {
      this->beginInsertRows(_qmi_for_node(parent), insert_at, insert_at);
   }
   node_ptr->parent = &parent;
   list.emplace_back(std::move(node_ptr));
   if (emit_signals) {
      this->endInsertRows();
   }
}
void IdleAnimationFormsModel::_reparent_child_idles(
   idle_parent_node&  parent_prior,
   idle_parent_node&  parent_after,
   const QModelIndex& parent_prior_qmi,
   const QModelIndex& parent_after_qmi
) {
   size_t dst_row = parent_after.children.size();
   this->beginMoveRows(
      parent_prior_qmi,
      0,
      parent_prior.children.size() - 1,
      parent_after_qmi,
      dst_row
   );
   parent_after.children.reserve(parent_after.children.size() + parent_prior.children.size());
   for (auto& idle_ptr : parent_prior.children) {
      auto* idle = idle_ptr.get();
      parent_after.children.push_back(std::move(idle_ptr));
      idle->parent = &parent_after;
      this->_update_idle_node_parent(*idle);
   }
   parent_prior.children.clear();
   this->endMoveRows();
}

IdleAnimationFormsModel::action_node* IdleAnimationFormsModel::_import_action(action_parent_node& parent, dovah::form_stub& stub, bool emit_signals) {
   auto&  list      = parent.children;
   size_t insert_at = list.size();

   QModelIndex parent_qmi;
   if (emit_signals) {
      parent_qmi = _qmi_for_node(parent);
      this->beginInsertRows(parent_qmi, insert_at, insert_at);
   }
   auto& node_ptr = parent.children.emplace_back(make_unique<action_node>());
   node_ptr->parent = &parent;
   node_ptr->stub   = &stub;
   node_ptr->update_cached_form_data();
   if (emit_signals) {
      this->endInsertRows();
   }
   return node_ptr.get();
}

void IdleAnimationFormsModel::import_idle(dovah::form_stub& idle, bool emit_signals) {
   if (idle.form_type != dovah::form_type::idle)
      return;
   {  // Check if we already have a node for this idle.
      auto& map = this->idle_forms_to_nodes;
      auto  it  = map.find(&idle);
      if (it != map.end())
         return;
   }
   auto loaded = idle.load().ptr_cast<dovah::loaded_forms::IdleAnimation>();
   if (!loaded)
      return;

   QString     graph_path     = QString::fromStdString(loaded->filename);
   graph_node* node_for_graph = _get_or_emplace_graph(graph_path, emit_signals);

   auto  node_for_idle_ptr = make_unique<idle_node>();
   auto& node_for_idle     = *node_for_idle_ptr.get();
   this->idle_forms_to_nodes[&idle] = &node_for_idle;

   if (loaded->parent) {
      auto* parent_form = loaded->parent.get_form_stub();
      switch (parent_form->form_type) {
         case dovah::form_type::action:
            {
               action_node* node_for_action = this->_node_for_action(node_for_graph, *parent_form);
               if (!node_for_action) {
                  auto* dst = node_for_graph ? (action_parent_node*)node_for_graph : this->nodes.loose.actions.get();
                  if (!node_for_graph)
                     dst = (this->nodes.loose.actions = make_unique<loose_action_parent_node>()).get();
                  //
                  node_for_action = _import_action(*dst, *parent_form, emit_signals);
               }
               this->_insert_idle_into(std::move(node_for_idle_ptr), *node_for_action, emit_signals);
            }
            break;
         case dovah::form_type::idle:
            {
               auto& map = this->idle_forms_to_nodes;
               auto  it  = map.find(parent_form);
               if (it == map.end()) {
                  this->import_idle(*parent_form, emit_signals);
                  it = map.find(parent_form);
               }
               if (it != map.end())
                  this->_insert_idle_into(std::move(node_for_idle_ptr), *it->second, emit_signals);
            }
            break;
         default:
            break;
      }
   }
   if (node_for_idle.parent)
      return;
   //
   // Insert IDLE as loose.
   //
   loose_idle_parent_node* loose_parent = nullptr;
   {
      auto& loose_ptr = node_for_graph ? node_for_graph->loose.idles : this->nodes.loose.idles;
      if (!loose_ptr) {
         if (emit_signals) {
            int row;
            if (node_for_graph) {
               row = node_for_graph->children.size();
            } else {
               row = this->nodes.graphs.size();
               if (this->nodes.loose.actions)
                  ++row;
            }
            this->beginInsertRows(
               node_for_graph ? _qmi_for_node(*node_for_graph) : _qmi_for_model_root(),
               row,
               row
            );
         }
         loose_ptr = make_unique<loose_idle_parent_node>();
         if (node_for_graph)
            loose_ptr->owner = node_for_graph;
         if (emit_signals) {
            this->endInsertRows();
         }
      }
   }
   this->_insert_idle_into(std::move(node_for_idle_ptr), *loose_parent, emit_signals);
}
void IdleAnimationFormsModel::import_all_idles() {
   this->beginResetModel();
   this->idle_forms_to_nodes.clear();
   this->nodes = {};
   DovahKitCore::get().for_each_form_of_type(dovah::form_type::idle, [this](dovah::form_stub* idle) {
      this->import_idle(*idle, false);
      return false;
   });
   this->endResetModel();
}

#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex IdleAnimationFormsModel::index(int row, int column, const QModelIndex& parent_qmi) const /*override*/ {
         if (row < 0 || column < 0)
            return {};
         const node* parent_node = _node_for_qmi(parent_qmi);
         const node* target_node = _child_node_by_row(parent_node, row);
         if (!target_node)
            return {};
         return _qmi_for_node(*target_node, column);
      }
      /*virtual*/ QModelIndex IdleAnimationFormsModel::parent(const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         const node* subject = _node_for_qmi(index);
         const node* parent  = nullptr;
         if (auto* casted = subject->as<node_type::action>())
            parent = casted->parent;
         else if (auto* casted = subject->as<node_type::idle>())
            parent = casted->parent;
         else if (auto* casted = subject->as<node_type::loose_action_container>())
            parent = casted->owner;
         else if (auto* casted = subject->as<node_type::loose_idle_container>())
            parent = casted->owner;
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
         if (auto* casted = basis->as<node_type::action>()) {
            if (!casted->parent)
               return {};
            if (!_child_node_by_row(casted->parent, row))
               return {};
            return _qmi_for_child_node(row, column, *casted->parent);
         } else if (auto* casted = basis->as<node_type::idle>()) {
            if (!casted->parent)
               return {};
            if (row >= casted->children.size())
               return {};
            return _qmi_for_child_node(row, column, *casted->parent);
         } else if (basis->as<node_type::graph>()) {
            basis_is_top_level = true;
         } else if (auto* casted = dynamic_cast<const loose_action_parent_node*>(basis)) {
            if (casted->owner) {
               return _qmi_for_child_node(row, column, *casted->owner);
            } else {
               basis_is_top_level = true;
            }
         } else if (auto* casted = dynamic_cast<const loose_idle_parent_node*>(basis)) {
            if (casted->owner) {
               return _qmi_for_child_node(row, column, *casted->owner);
            } else {
               basis_is_top_level = true;
            }
         }
         if (basis_is_top_level) {
            const size_t size = this->nodes.graphs.size();
            if (row < size)
               return _qmi_for_node(*this->nodes.graphs[row], column);
            node* subject = nullptr;
            if (row == size) {
               subject = this->nodes.loose.actions.get();
               if (!subject)
                  subject = this->nodes.loose.idles.get();
            } else if (row == size + 1) {
               if (this->nodes.loose.actions)
                  subject = this->nodes.loose.idles.get();
            }
            if (subject)
               return _qmi_for_node(*subject, column);
         }
         return {};
      }
      /*virtual*/ int IdleAnimationFormsModel::rowCount(const QModelIndex& parent) const /*override*/ {
         const auto* node = _node_for_qmi(parent);
         if (!node)
            return 0;
         size_t size = 0;
         if (auto* casted = dynamic_cast<const action_parent_node*>(node)) {
            size = casted->children.size();
            if (auto* subtype = casted->as<node_type::graph>())
               if (subtype->loose.idles)
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
         const auto* node = _node_for_qmi(qmi);
         if (!node)
            return {};
         switch (role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
               if (auto* casted = node->as<node_type::graph>()) {
                  return casted->path;
               } else if (auto* casted = node->as<node_type::action>()) {
                  return casted->cached.editor_id;
               } else if (auto* casted = node->as<node_type::idle>()) {
                  return casted->cached.editor_id;
               } else if (auto* casted = node->as<node_type::loose_action_container>()) {
                  return tr("LOOSE BY ACTION");
               } else if (auto* casted = node->as<node_type::loose_idle_container>()) {
                  return tr("LOOSE");
               } else {
                  return tr("???");
               }
               break;
            case Qt::ForegroundRole:
               if (auto* casted = node->as<node_type::action>()) {
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
         if (auto* casted = node->as<node_type::action>()) {
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