#include "./graph_node.h"

namespace IdleAnimationFormsModel_impl {
   graph_node::graph_node() : node(node_type::graph) {
      this->children.loose = std::make_unique<loose_container_node>();
      this->children.loose->parent = this;
   }

   /*virtual*/ size_t graph_node::index_of_child(const node& child) const /*override*/ {
      if (child.parent != this)
         return index_of_none;
      const auto&  list = this->children.actions;
      const size_t size = list.size();
      if (&child == this->children.loose.get())
         return size;
      for (size_t i = 0; i < size; ++i) {
         if (list[i].get() == &child)
            return i;
      }
      assert(false && "A node claims to be our child, but isn't in our child list!");
      return index_of_none;
   }
   /*virtual*/ const node* graph_node::nth_child(size_t n) const /*override*/ {
      const auto&  list = this->children.idles;
      const size_t size = list.size();
      if (n == size)
         return this->children.loose.get();
      else if (n > size)
         return nullptr;
      return list[n].get();
   }
   /*virtual*/ void graph_node::update_cached_form_data() /*override*/ {
   }
}