#pragma once
#include "./_parent_node_mixin.h"
#include <cassert>

#pragma push_macro("CLASS_PARAMS")
#pragma push_macro("CLASS_NAME")
#define CLASS_PARAMS template<typename ChildNode>
#define CLASS_NAME _parent_node_mixin<ChildNode>

namespace IdleAnimationFormsModel_impl {
   CLASS_PARAMS
   constexpr size_t CLASS_NAME::index_of_child(const child_node_type& child) const noexcept {
      if (child.parent != this)
         return no_index;
      for (size_t i = 0; i < this->children.size(); ++i)
         if (this->children[i].get() == &child)
            return i;
      assert(false && "If a child node claims that we're its parent, then it should be in our child list!");
      return no_index;
   }

   CLASS_PARAMS
   constexpr size_t CLASS_NAME::index_of_form(dovah::form_stub& stub) const noexcept requires impl::node_for_form<child_node_type> {
      for (size_t i = 0; i < this->children.size(); ++i)
         if (this->children[i]->stub == &stub)
            return i;
      return no_index;
   }

   CLASS_PARAMS
   constexpr node_unique_ptr<typename CLASS_NAME::child_node_type> CLASS_NAME::take_child(size_t i) {
      assert(i < this->children.size());
      auto taken = std::move(this->children[i]);
      taken->parent = nullptr;
      this->children.erase(this->children.begin() + i);
      return taken;
   }

}
#undef CLASS_PARAMS
#undef CLASS_NAME