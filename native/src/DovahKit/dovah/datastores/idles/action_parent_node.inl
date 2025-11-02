#pragma once
#include "./action_parent_node.h"

namespace dovah::datastores::impl::idles {
   constexpr size_t action_parent_node::index_of_child(const action_node& action) const noexcept {
      for (size_t i = 0; i < this->children.size(); ++i) {
         const action_node* node = this->children[i];
         if (node == &action)
            return i;
      }
      return no_index;
   }
}