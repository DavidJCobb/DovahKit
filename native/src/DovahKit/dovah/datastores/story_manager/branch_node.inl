#pragma once
#include "./branch_node.h"

namespace dovah::datastores::impl::story_manager {
   constexpr bool branch_node::is_ancestor_of(const node& n) const noexcept {
      const branch_node* parent = n.parent;
      if (!parent)
         return false;
      do {
         if (parent == this)
            return true;
      } while (parent = parent->parent);
      return false;
   }
   constexpr size_t branch_node::index_of_child(const form_stub& stub) const noexcept {
      for (size_t i = 0; i < this->children.size(); ++i)
         if (&this->children[i]->stub == &stub)
            return i;
      return index_of_none;
   }
   constexpr size_t branch_node::index_of_child(const node& n) const noexcept {
      for (size_t i = 0; i < this->children.size(); ++i)
         if (this->children[i] == &n)
            return i;
      return index_of_none;
   }
}