#pragma once
#include <memory>
#include <vector>
#include "./_base_node.h"
#include "./_parent_node_mixin.h"
namespace IdleAnimationFormsModel_impl {
   class action_node;
}

namespace IdleAnimationFormsModel_impl {
   class action_parent_node : public node, public _parent_node_mixin<action_node> {
      protected:
         constexpr action_parent_node(node_type t) : node(t) {}
      public:
         ~action_parent_node();
   };

   class loose_action_parent_node final : public action_parent_node {
      public:
         constexpr loose_action_parent_node() : action_parent_node(node_type::loose_action_container) {}

      public:
         node* owner = nullptr;
   };
}