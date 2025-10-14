#include "./idle_node.h"
#include "dovah/form_stub.h"

namespace IdleAnimationFormsModel_impl {
   /*virtual*/ size_t idle_node::index_of_child(const node& child) const /*override*/ {
      if (child.parent != this)
         return index_of_none;
      assert(child.type == node_type::idle && "A node claims to be our child, but is of the wrong type!");
      const auto&  list = this->children.idles;
      const size_t size = list.size();
      for (size_t i = 0; i < size; ++i) {
         if (list[i].get() == &child)
            return i;
      }
      assert(false && "A node claims to be our child, but isn't in our child list!");
      return index_of_none;
   }
   /*virtual*/ const node* idle_node::nth_child(size_t n) const /*override*/ {
      const auto& list = this->children.idles;
      if (n >= list.size())
         return nullptr;
      return list[n].get();
   }
   /*virtual*/ void idle_node::update_cached_form_data() /*override*/ {
      if (this->stub == nullptr) {
         this->cached.editor_id = QString("NONE");
         return;
      }
      this->cached.editor_id = QString::fromStdString(this->stub->get_editor_id());
   }
}