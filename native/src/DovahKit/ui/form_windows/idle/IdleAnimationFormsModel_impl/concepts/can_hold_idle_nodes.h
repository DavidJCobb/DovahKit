#pragma once
#include <concepts>
#include <memory>
#include <type_traits>
#include <vector>
namespace IdleAnimationFormsModel_impl {
   class node;
   class idle_node;
}

namespace IdleAnimationFormsModel_impl::concepts {
   template<typename ParentNode>
   concept can_hold_idle_nodes = requires(ParentNode& parent) {
      requires std::is_base_of_v<node, ParentNode>;
      { parent.children.idles } -> std::same_as<std::vector<std::unique_ptr<idle_node>>&>;
   };
}