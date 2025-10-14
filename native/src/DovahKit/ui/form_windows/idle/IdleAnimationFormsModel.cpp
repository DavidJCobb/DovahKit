#include "./IdleAnimationFormsModel.h"
#include "./IdleAnimationFormsModel_impl/action_node.h"
#include "./IdleAnimationFormsModel_impl/idle_node.h"
#include "dovah/forms/IdleAnimation.h"
#include "dovah/form_stub.h"

#include "./IdleAnimationFormsModel_impl/utils/insert_sorted_child.h"
namespace {
   namespace node_utils {
      using namespace IdleAnimationFormsModel_impl::utils;
   }
}

IdleAnimationFormsModel::IdleAnimationFormsModel(QObject* parent) : QAbstractItemModel(parent) {
   this->nodes.loose = std::make_unique<loose_container_node>();
}

#pragma region Node utils
   QModelIndex IdleAnimationFormsModel::_qmi_for_model_root() const {
      return {};
   }
   QModelIndex IdleAnimationFormsModel::_qmi_for_node(const node& n, int column) const {
      if (column < 0)
         return {};
      int   row    = -1;
      auto* parent = n.parent;
      if (parent) {
         row = parent->index_of_child(n);
      } else {
         const auto&  list = this->nodes.graphs;
         const size_t size = list.size();
         if (&n == this->nodes.loose.get())
            row = size;
         else {
            for (size_t i = 0; i < size; ++i) {
               if (list[i].get() == &n) {
                  row = i;
                  break;
               }
            }
         }
      }
      if (row == -1)
         return {};
      return this->createIndex(row, column, (quintptr)parent);
   }
   const IdleAnimationFormsModel::node* IdleAnimationFormsModel::_node_for_qmi(const QModelIndex& qmi) const {
      if (!qmi.isValid())
         return nullptr;
      int   i      = qmi.row();
      auto* parent = (node*)qmi.internalPointer();
      if (parent) {
         return parent->nth_child(i);
      } else {
         const auto&  list = this->nodes.graphs;
         const size_t size = list.size();
         if (i == size) {
            return this->nodes.loose.get();
         } else if (i < size) {
            return list[i].get();
         }
      }
      return nullptr;
   }
   IdleAnimationFormsModel::node* IdleAnimationFormsModel::_node_for_qmi(const QModelIndex& qmi) {
      return const_cast<node*>(std::as_const(*this)._node_for_qmi(qmi));
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
      for (auto& node_ptr : graph.children.actions)
         if (node_ptr->stub == &stub)
            return node_ptr.get();
      return nullptr;
   }
   IdleAnimationFormsModel::action_node* IdleAnimationFormsModel::_node_for_loose_action(const dovah::form_stub& stub) {
      for (auto& node_ptr : this->nodes.loose->children.actions)
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
   void IdleAnimationFormsModel::_on_form_created(dovah::form_stub& stub);
   void IdleAnimationFormsModel::_on_form_modified(dovah::form_stub& stub);
   void IdleAnimationFormsModel::_on_form_deletion_imminent(dovah::form_stub& stub, bool just_being_flagged) {
      auto _reparent_child_idles = [this]<
         typename ParentPrior,
         typename ParentAfter
      >(
         ParentPrior& parent_prior,
         ParentAfter& parent_after,
         const QModelIndex& parent_prior_qmi,
         const QModelIndex& parent_after_qmi
      ) {
         size_t dst_row = parent_after.children.idles.size();
         if constexpr (requires(ParentAfter& p) {
            { p.children.actions };
         }) {
            dst_row += parent_after.children.actions.size();
         }

         this->beginMoveRows(
            parent_prior_qmi,
            0,
            parent_prior.children.idles.size() - 1,
            parent_after_qmi,
            dst_row
         );
         parent_after.children.idles.reserve(parent_after.children.idles.size() + parent_prior.children.idles.size());
         for (auto& idle_ptr : parent_prior.children.idles) {
            auto* idle = idle_ptr.get();
            parent_after.children.idles.push_back(std::move(idle_ptr));
            idle->parent = &parent_after;
            this->_update_idle_node_parent(*idle);
         }
         parent_prior.children.idles.clear();
         this->endMoveRows();
      };

      switch (stub.form_type) {
         case dovah::form_type::action:
            {
               auto functor = [this, &stub, just_being_flagged, &_reparent_child_idles]<typename ActionParent, typename LooseIdleParent>(ActionParent& parent, LooseIdleParent& loose_node) {
                  auto&  action_list  = parent.children.actions;
                  size_t action_count = action_list.size();
                  for (size_t i = 0; i < action_count; ++i) {
                     std::unique_ptr<action_node>& action_ptr = action_list[i];
                     if (action_ptr->stub != &stub)
                        continue;
                     auto parent_qmi = _qmi_for_node(parent);
                     _reparent_child_idles(
                        *action_ptr,
                        loose_node,
                        _qmi_for_node(*action_ptr),
                        _qmi_for_node(loose_node)
                     );
                     if (!just_being_flagged) {
                        //
                        // Delete the action node.
                        //
                        this->beginRemoveRows(parent_qmi, i, i);
                        action_list.erase(action_list.begin() + i);
                        --i;
                        --action_count;
                        this->endRemoveRows();
                        continue;
                     }
                  }
               };
               functor(*this->nodes.loose, *this->nodes.loose);
               for (auto& graph_ptr : this->nodes.graphs) {
                  functor(*graph_ptr, *graph_ptr->children.loose);
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
                  if (auto* casted = parent_node->as<node_type::action>()) {
                     _reparent_child_idles(
                        *idle_as_node,
                        *casted,
                        _qmi_for_node(*idle_as_node),
                        _qmi_for_node(*casted)
                     );
                  } else if (auto* casted = parent_node->as<node_type::loose_container>()) {
                     _reparent_child_idles(
                        *idle_as_node,
                        *casted,
                        _qmi_for_node(*idle_as_node),
                        _qmi_for_node(*casted)
                     );
                  }
                  //
                  auto _remove_idle = [this, idle_as_node]<typename ParentNode>(ParentNode& parent) {
                     auto& list = parent.children.idles;
                     auto  it   = std::find(list.begin(), list.end(), idle_as_node);
                     if (it != list.end()) {
                        size_t i = std::distance(list.begin(), it);
                        this->beginRemoveRows(_qmi_for_node(parent), i, i);
                        list.erase(it);
                        this->endRemoveRows();
                     }
                  };
                  if (auto* casted = parent_node->as<node_type::action>()) {
                     _remove_idle(*casted);
                  } else if (auto* casted = parent_node->as<node_type::loose_container>()) {
                     _remove_idle(*casted);
                  }
               }
            }
            break;
      }
   }
#pragma endregion

IdleAnimationFormsModel::action_node* IdleAnimationFormsModel::_import_action(graph_node& parent, dovah::form_stub& stub, bool emit_signals) {
   auto&  list      = parent.children.actions;
   size_t insert_at = list.size();

   QModelIndex parent_qmi;
   if (emit_signals) {
      parent_qmi = _qmi_for_node(parent);
      this->beginInsertRows(parent_qmi, insert_at, insert_at);
   }
   auto& node_ptr = parent.children.actions.emplace_back(std::make_unique<action_node>());
   node_ptr->parent = &parent;
   node_ptr->stub   = &stub;
   node_ptr->update_cached_form_data();
   if (emit_signals) {
      this->endInsertRows();
   }
   return node_ptr.get();
}
IdleAnimationFormsModel::action_node* IdleAnimationFormsModel::_import_action(loose_container_node& parent, dovah::form_stub& stub, bool emit_signals) {
   auto&  list      = parent.children.actions;
   size_t insert_at = list.size();

   QModelIndex parent_qmi;
   if (emit_signals) {
      parent_qmi = _qmi_for_node(parent);
      this->beginInsertRows(parent_qmi, insert_at, insert_at);
   }
   auto& node_ptr = parent.children.actions.emplace_back(std::make_unique<action_node>());
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
   graph_node* node_for_graph = this->_node_for_graph_path(graph_path);
   if (!node_for_graph) {
      size_t insert_at = this->nodes.graphs.size(); // TODO: sort alphabetically by path segments
      if (emit_signals) {
         this->beginInsertRows({}, insert_at, insert_at);
      }
      auto& node_ptr = this->nodes.graphs.emplace_back(std::make_unique<graph_node>());
      auto& node     = *node_ptr.get();
      node.path = graph_path;
      if (emit_signals) {
         this->endInsertRows();
      }
   }

   auto  node_for_idle_ptr = std::make_unique<idle_node>();
   auto& node_for_idle     = *node_for_idle_ptr.get();
   this->idle_forms_to_nodes[&idle] = &node_for_idle;

   auto  _insert_into_parent = [this, emit_signals, &node_for_idle_ptr]<typename Parent>(Parent& parent) {
      auto&  list      = parent.children.idles;
      size_t insert_at = list.size(); // TODO: sort alphabetically by path segments
      if (emit_signals) {
         this->beginInsertRows(_qmi_for_node(parent), insert_at, insert_at);
      }
      node_for_idle_ptr->parent = &parent;
      list.push_back(std::move(node_for_idle_ptr));
      if (emit_signals) {
         this->endInsertRows();
      }
   };

   if (loaded->parent) {
      auto* parent_form = loaded->parent.get_form_stub();
      switch (parent_form->form_type) {
         case dovah::form_type::action:
            {
               action_node* node_for_action = this->_node_for_action(node_for_graph, *parent_form);
               if (!node_for_action) {
                  if (node_for_graph) {
                     node_for_action = _import_action(*node_for_graph, *parent_form, emit_signals);
                  } else {
                     node_for_action = _import_action(*this->nodes.loose, *parent_form, emit_signals);
                  }
               }
               _insert_into_parent(*node_for_action);
            }
            break;
         case dovah::form_type::idle:
            {
               node* parent_node = nullptr;
               {
                  auto& map = this->idle_forms_to_nodes;
                  auto  it  = map.find(parent_form);
                  if (it == map.end()) {
                     this->import_idle(*parent_form, emit_signals);
                     it = map.find(parent_form);
                  }
                  if (it != map.end()) {
                     _insert_into_parent(*it->second);
                  } else {
                     _insert_into_parent(*this->nodes.loose);
                  }
               }
            }
            break;
         default:
            if (node_for_graph)
               _insert_into_parent(*node_for_graph->children.loose);
            else
               _insert_into_parent(*this->nodes.loose);
      }
   } else {
      _insert_into_parent(*node_for_graph->children.loose);
   }
}