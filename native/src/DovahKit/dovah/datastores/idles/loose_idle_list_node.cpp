#include "./loose_idle_list_node.h"
#include <cassert>
#include "helpers/vectors/re_sort_item_within.h"
#include "./idle_node.h"
#include "./passkeys/post_build_edit.h"
#include "../../form_stub.h"

namespace dovah::datastores::impl::idles {
   /*static*/ bool loose_idle_list_node::idle_sort_comparator(const idle_node* a, const idle_node* b) {
      const std::string_view name_a = a->stub.get_editor_id();
      const std::string_view name_b = b->stub.get_editor_id();
      const auto min_size = std::min(name_a.size(), name_b.size());
      for (size_t i = 0; i < min_size; ++i) {
         char ca = name_a[i];
         char cb = name_b[i];
         if (ca >= 'A' && ca <= 'Z')
            ca += 0x20;
         if (cb >= 'A' && cb <= 'Z')
            cb += 0x20;
         if (ca != cb)
            return ca < cb;
      }
      return name_b.size() > min_size;
   }

   size_t loose_idle_list_node::index_of_child(const idle_node& idle) const noexcept {
      for (size_t i = 0; i < this->child_idles.size(); ++i)
         if (this->child_idles[i] == &idle)
            return i;
      return index_of_none;
   }

   size_t loose_idle_list_node::prospective_index_of(const idle_node& idle) const noexcept {
      auto it = std::upper_bound(
         this->child_idles.begin(),
         this->child_idles.end(),
         &idle,
         &idle_sort_comparator
      );
      return std::distance(this->child_idles.begin(), it);
   }

   void loose_idle_list_node::re_sort_idle(passkeys::post_build_edit, size_t i) {
      assert(i < this->child_idles.size());
      cobb::vectors::re_sort_item_within(
         this->child_idles,
         this->child_idles.begin() + i,
         &idle_sort_comparator
      );
   }
}