#include "./loose_container_node.h"

namespace IdleAnimationFormsModel_impl {
   /*virtual*/ size_t loose_container_node::index_of_child(const node& child) const /*override*/ {
      if (child.parent != this)
         return index_of_none;
      if (child.type == node_type::action) {
         const auto&  list = this->children.actions;
         const size_t size = list.size();
         for (size_t i = 0; i < size; ++i) {
            if (list[i].get() == &child)
               return i;
         }
      } else if (child.type == node_type::idle) {
         const size_t count_before = this->children.actions.size();
         const auto&  list = this->children.idles;
         const size_t size = list.size();
         for (size_t i = 0; i < size; ++i) {
            if (list[i].get() == &child)
               return count_before + i;
         }
      } else {
         assert(false && "A node claims to be our child, but is of the wrong type!");
      }
      assert(false && "A node claims to be our child, but isn't in our child list!");
      return index_of_none;
   }
   /*virtual*/ const node* loose_container_node::nth_child(size_t n) const /*override*/ {
      const auto& list = this->children.idles;
      if (n >= list.size())
         return nullptr;
      return list[n].get();
   }
}