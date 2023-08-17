#pragma once

namespace cobb::index_sequences {
   namespace impl {
      template<size_t By, typename Sequence>
      struct _increment_type;

      template<size_t By, size_t... Indices>
      struct _increment_type<By, std::index_sequence<Indices...>> {
         using type = std::index_sequence<By + Indices...>;
      };
   }

   template<typename Sequence, size_t By>
   using incremented_by = typename impl::_increment_type<By, Sequence>::type;
}