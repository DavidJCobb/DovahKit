#pragma once
#include "../concepts/can_hold_action_nodes.h"
#include "../concepts/can_hold_idle_nodes.h"

namespace IdleAnimationFormsModel_impl::utils {
   template<typename ChildType, typename ParentType>
      requires (!std::is_same_v<ParentType, node>)
   constexpr auto& get_child_list(ParentType& parent) {
      if constexpr (std::is_same_v<ChildType, action_node>) {
         if constexpr (concepts::can_hold_action_nodes<ParentType>) {
            return parent.children.actions;
         }
      } else if constexpr (std::is_same_v<ChildType, idle_node>) {
         if constexpr (concepts::can_hold_idle_nodes<ParentType>) {
            return parent.children.idles;
         }
      }
      return;
   }
}