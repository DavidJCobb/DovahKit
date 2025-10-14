#pragma once
#include <concepts>
#include <memory>
#include <type_traits>
#include <vector>
namespace IdleAnimationFormsModel_impl {
   class node;
   class action_node;
}

namespace IdleAnimationFormsModel_impl::concepts {
   template<typename ParentNode>
   concept can_hold_action_nodes = requires(ParentNode& parent) {
      requires std::is_base_of_v<node, ParentNode>;
      { parent.children.actions } -> std::same_as<std::vector<std::unique_ptr<action_node>>&>;
   };
}