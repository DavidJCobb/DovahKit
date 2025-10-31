#pragma once
#include <cassert>
#include "idle_tree_builder.h"

#pragma push_macro("CLASS_TEMPLATE_PARAMS")
#pragma push_macro("CLASS_NAME")
#define CLASS_TEMPLATE_PARAMS template<typename GraphNode, typename ActionNode, typename IdleNode> requires impl::idle_tree_builder::node_requirements<GraphNode, ActionNode, IdleNode>
#define CLASS_NAME idle_tree_builder<GraphNode, ActionNode, IdleNode>

namespace dovah::utils {
   CLASS_TEMPLATE_PARAMS
   CLASS_NAME::idle_tree_builder() {
      this->loose.actions = new action_parent_node;
      this->loose.idles   = new loose_idle_parent_node;
   }

   CLASS_TEMPLATE_PARAMS
   CLASS_NAME::~idle_tree_builder() {
      auto _delete_list = [](auto& list) {
         for (auto*& graph : list) {
            if (!graph)
               continue;
            delete graph;
            graph = nullptr;
         }
         list.clear();
      };

      this->idles_by_stub.clear();
      this->cyclical_idles.clear();
      _delete_list(this->graphs);
      {
         auto*& ptr = this->loose.actions;
         if (ptr) {
            delete ptr;
            ptr = nullptr;
         }
      }
      {
         auto*& ptr = this->loose.idles;
         if (ptr) {
            delete ptr;
            ptr = nullptr;
         }
      }
   }

   CLASS_TEMPLATE_PARAMS
   typename CLASS_NAME::idle_node& CLASS_NAME::_get_or_create_idle_node(loaded_idle_type& loaded_idle, idle_parent_node& in_parent) {
      idle_node*& mapping = this->idles_by_stub[&loaded_idle.stub];
      if (mapping)
         return *mapping;
      auto* idle = new idle_node(loaded_idle);
      try {
         in_parent.append_child(*idle);
      } catch (...) {
         if (idle)
            delete idle;
         throw;
      }
      mapping = idle;
      return *idle;
   }

   CLASS_TEMPLATE_PARAMS
   typename CLASS_NAME::idle_node& CLASS_NAME::_get_or_create_idle_node(loaded_idle_type& loaded_idle) {
      return this->_get_or_create_idle_node(loaded_idle, *this->loose.idles);
   }

   CLASS_TEMPLATE_PARAMS
   void CLASS_NAME::_build_parent_idle(loaded_idle_type& loaded_idle) {
      assert(loaded_idle.data.flags & idle_flag::parent);
      auto*      parent           = loaded_idle.parent.get_form_stub();
      const bool parent_is_action = parent && parent->form_type == form_type::action;

      idle_parent_node* parent_node = nullptr;

      auto* graph = this->get_or_create_graph_by_path(loaded_idle.filename);
      if (graph) {
         if (parent_is_action) {
            parent_node = graph->get_or_create_action(*parent);
         } else {
            parent_node = graph->loose;
            assert(parent_node != nullptr);
         }
      } else if (parent_is_action) {
         parent_node = this->get_or_create_loose_action(*parent);
         assert(parent_node != nullptr);
      }
      if (!parent_node) {
         parent_node = this->loose.idles;
         assert(parent_node != nullptr);
      }
      this->_get_or_create_idle_node(loaded_idle, *parent_node);
   }

   CLASS_TEMPLATE_PARAMS
   void CLASS_NAME::_build_child_idle(loaded_idle_type& loaded_idle) {
      auto* graph = graph_by_path(loaded_idle.filename);

      dovah::form_stub* parent   = loaded_idle.parent.get_form_stub();
      dovah::form_stub* previous = loaded_idle.previous_sibling.get_form_stub();
      dovah::form_stub* action   = nullptr;
      if (parent && parent->form_type != form_type::idle) {
         if (parent->form_type == form_type::action)
            action = parent;
         parent = nullptr;
      }

      idle_parent_node* parent_node = nullptr;

      seen_idle_set seen;
      if (this->_has_cyclical_parentage(seen, loaded_idle)) {
         error("Infinite loop detected in init for %s.", stub);
         parent   = nullptr;
         previous = nullptr;
      }
      {
         auto problem = this->_check_siblings(seen, loaded_idle);
         if (problem.has_value()) {
            if (*problem == sibling_problem::cyclical) {
               error("Infinite loop detected in init for %s.", loaded_idle.stub);
            } else if (*problem == sibling_problem::mismatched) {
               error("Invalid prev idle '%s' (%08X) detected in init for %s.", current, current->formID, loaded_idle.stub);
            }
            if (!parent_node && parent) {
               auto* g = this->graph_by_idle(*parent);
               if (g)
                  parent_node = g->loose;
            }
            if (!parent_node && previous) {
               auto* g = this->graph_by_idle(*previous);
               if (g)
                  parent_node = g->loose;
            }
            parent   = nullptr;
            previous = nullptr;
         }
      }

      auto& idle_node = this->_get_or_create_idle_node(loaded_idle);
      if (parent) {
         auto loaded_parent = parent->load().ptr_cast<loaded_idle_type>();
         if (loaded_parent) {
            this->_get_or_create_idle_node(*loaded_parent).append_child(idle_node);
            return;
         }
         parent = nullptr;
      }
      if (action) {
         if (graph) {
            parent_node = graph->get_or_create_action(*action);
         } else {
            parent_node = this->get_or_create_loose_action(*action);
         }
         assert(parent_node != nullptr);
      } else {
         error("Idle '%s' has no parent and is not an action root.", loaded_idle.stub);
      }
      if (!parent_node) {
         parent_node = this->loose.idles;
         assert(parent_node != nullptr);
      }
      parent_node->append_child(idle_node);
   }

   CLASS_TEMPLATE_PARAMS
   bool CLASS_NAME::_has_cyclical_parentage(seen_idle_set& seen, const loaded_idle_type& idle) {
      auto* current = idle.parent.get_form_stub();
      do {
         if (!current)
            break;
         if (current->form_type != dovah::form_type::idle)
            break;
         if (seen.contains(current))
            return true;
         seen.insert(current);

         auto loaded = current->load().ptr_cast<loaded_idle_type>();
         if (!loaded)
            break;
         current = loaded->parent.get_form_stub();
      } while (true);
      return false;
   }

   CLASS_TEMPLATE_PARAMS
   std::optional<typename CLASS_NAME::sibling_problem> CLASS_NAME::_check_siblings(seen_idle_set& seen, const loaded_idle_type& idle) {
      for (auto* current = previous; current; current = current->previous_sibling) {
         if (seen.contains(current))
            return sibling_problem::cyclical;
         if (current->parent != idle.parent.get_form_stub())
            return sibling_problem::mismatched;
         seen.insert(current);
      }
      return {};
   }

   CLASS_TEMPLATE_PARAMS
   void CLASS_NAME::build(file_load_order& lo) {
      lo.for_each_form_of_type(form_type::idle, [](dovah::form_stub* stub) -> bool {
         auto loaded = stub->load().ptr_cast<loaded_idle_type>();
         if (!loaded)
            return false;

         auto* parent = loaded->parent.get_form_stub();
         if (loaded->data.flags & idle_flag::parent) {
            this->_build_parent_idle(loaded);
            return false;
         }
         this->_build_child_idle(loaded);
         return false;
      });
      for (auto* graph : this->graphs) {
         graph->sort_descendants();
      }
      for (auto* action : this->loose.actions) {
         action->sort_descendants();
      }
      for (auto* idle : this->loose.idles->children) {
         idle->sort_descendants();
      }
   }

   #pragma region Graph node getters
      CLASS_TEMPLATE_PARAMS
      const typename CLASS_NAME::graph_node* CLASS_NAME::graph_by_idle(dovah::form_stub& stub) const noexcept {
         if (stub.form_type != form_type::idle)
            return nullptr;
         auto loaded = stub.load().ptr_cast<loaded_idle_type>();
         if (!loaded)
            return nullptr;
         return this->graph_by_path(loaded.filename);
      }

      CLASS_TEMPLATE_PARAMS
      typename CLASS_NAME::graph_node* CLASS_NAME::graph_by_idle(dovah::form_stub& stub) noexcept {
         return const_cast<graph_node*>(std::as_const(*this).graph_by_idle(stub));
      }

      CLASS_TEMPLATE_PARAMS
      const typename CLASS_NAME::graph_node* CLASS_NAME::graph_by_path(std::string_view path) const noexcept {
         for (auto* graph : this->graphs)
            if (graph->path == path) // TODO: Case-insensitive comparison
               return graph;
         return nullptr;
      }

      CLASS_TEMPLATE_PARAMS
      typename CLASS_NAME::graph_node* CLASS_NAME::graph_by_path(std::string_view path) noexcept {
         return const_cast<graph_node*>(std::as_const(*this).graph_by_path(path));
      }
   
      CLASS_TEMPLATE_PARAMS
      typename CLASS_NAME::graph_node* CLASS_NAME::get_or_create_graph_by_path(std::string_view path) {
         auto* graph = this->graph_by_path(path);
         if (graph)
            return graph;
         auto& pointer = this->graphs.emplace_back();
         pointer = new graph_node;
         pointer->path = path;
         return pointer;
      }
   #pragma endregion

   #pragma region Loose action getters
      CLASS_TEMPLATE_PARAMS
      const typename CLASS_NAME::action_node* CLASS_NAME::loose_action(dovah::form_stub& stub) const noexcept {
         return this->loose.actions->get_action(stub);
      }

      CLASS_TEMPLATE_PARAMS
      typename CLASS_NAME::action_node* CLASS_NAME::loose_action(dovah::form_stub& stub) noexcept {
         return const_cast<action_node*>(std::as_const(*this).loose_action(stub));
      }
      
      CLASS_TEMPLATE_PARAMS
      typename CLASS_NAME::action_node* CLASS_NAME::get_or_create_loose_action(dovah::form_stub& stub) {
         return this->loose.actions->get_or_create_action(stub);
      }
   #pragma endregion
}

#undef CLASS_TEMPLATE_PARAMS
#undef CLASS_NAME
#pragma pop_macro("CLASS_TEMPLATE_PARAMS")
#pragma pop_macro("CLASS_NAME")