#pragma once
#include "./concepts/storage_is_bifurcated.h"

namespace dovahscript::api_helpers::native_lists::impl {
   // Takes: wrapper; zero-based index within list
   // Returns: list to modify; zero-based index within list
   template<typename Spec>
   std::pair<typename Spec::collection_wrapped_type*, size_t> get_list_read_target(wrapper& self, size_t requested_index) {
      if constexpr (!impl::concepts::storage_is_bifurcated<Spec>) {
         return std::pair{ Spec::unwrap_collection(self), requested_index };
      } else {
         const auto pair = Spec::unwrap_collection(self);
         if (pair.first) {
            const size_t split_at = pair.first->size();
            if (requested_index < split_at)
               return { pair.first, requested_index };
            requested_index -= split_at;
         }
         return { pair.second, requested_index };
      }
   }
}