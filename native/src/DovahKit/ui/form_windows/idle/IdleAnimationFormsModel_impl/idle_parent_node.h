#pragma once
#include <memory>
#include <vector>
#include "./_base_node.h"
#include "./_parent_node_mixin.h"
namespace IdleAnimationFormsModel_impl {
   class idle_node;
}

namespace IdleAnimationFormsModel_impl {
   class idle_parent_node : public node, public _parent_node_mixin<idle_node> {
      protected:
         constexpr idle_parent_node(node_type t) : node(t) {}
      public:
         ~idle_parent_node();
   };

   class loose_idle_parent_node final : public idle_parent_node {
      public:
         constexpr loose_idle_parent_node() : idle_parent_node(node_type::loose_idle_container) {}

      public:
         node* owner = nullptr;
   };
}