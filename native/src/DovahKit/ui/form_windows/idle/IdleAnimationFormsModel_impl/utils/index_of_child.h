#pragma once
#include "../concepts/can_hold_action_nodes.h"
#include "../concepts/can_hold_idle_nodes.h"
#include "../node_type.h"

namespace IdleAnimationFormsModel_impl::utils {
   template<typename ParentNode>
   size_t index_of_child(const ParentNode& parent, const action_node& child) {
      if constexpr (concepts::can_hold_action_nodes<ParentNode>) {
         const auto&  list = parent.children.actions;
         const size_t size = list.size();
         for (size_t i = 0; i < size; ++i)
            if (list[i].get() == &child)
               return i;
      }
      return (size_t)-1;
   }
   template<typename ParentNode>
   size_t index_of_child(const ParentNode& parent, const idle_node& child) {
      if constexpr (concepts::can_hold_idle_nodes<ParentNode>) {
         const auto&  list = parent.children.idles;
         const size_t size = list.size();
         for (size_t i = 0; i < size; ++i)
            if (list[i].get() == &child)
               return i;
      }
      return (size_t)-1;
   }

   template<typename ParentNode>
   size_t index_of_child(const ParentNode& parent, const node& child) {
      switch (child.type) {
         case node_type::action:
            return index_of_child(parent, (action_node&)child);
         case node_type::idle:
            return index_of_child(parent, (idle_node&)child);
      }
      return (size_t)-1;
   }
}