#include "./loose_idle_list_node.h"
#include "./idle_node.h"
#include "../../form_stub.h"

namespace dovah::datastores::impl::idles2 {
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
         [](const idle_node* a, const idle_node* b) -> bool {
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
      );
      return std::distance(this->child_idles.begin(), it);
   }
}