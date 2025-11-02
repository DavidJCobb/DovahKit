#pragma once
#include "./idle_parent_node.h"

namespace dovah::datastores::impl::idles {
   constexpr size_t idle_parent_node::index_of_child(const idle_node& idle) const noexcept {
      for (size_t i = 0; i < this->children.size(); ++i) {
         const idle_node* node = this->children[i];
         if (node == &idle)
            return i;
      }
      return no_index;
   }
}