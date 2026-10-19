#pragma once
#include <utility>
#include "./intersection.h"

namespace lu::utils::bool_sequences {
   namespace impl {
      template<typename Sequence, std::size_t From>
      struct _clear_all_from;

      // shortcut for out-of-bounds clear
      template<bool... B, std::size_t From>
         requires (From >= sizeof...(B))
      struct _clear_all_from<std::integer_sequence<bool, B...>, From> {
         using type = std::integer_sequence<bool, B...>;
      };

      template<bool... B, std::size_t From>
         requires (From < sizeof...(B))
      struct _clear_all_from<std::integer_sequence<bool, B...>, From> {
         using sequence = std::integer_sequence<bool, B...>;

         template<typename>
         struct all_true_until;
         //
         template<size_t... I>
         struct all_true_until<std::index_sequence<I...>> {
            using type = std::integer_sequence<bool, (I < From)...>;
         };

         using type = intersection<
            sequence,
            typename all_true_until<std::make_index_sequence<sizeof...(B)>>::type
         >;
      };
   }

   // Given a `std::integer_sequence<bool, ...>` type, create a new bool sequence wherein all 
   // NTTPs at and past index `From` are `false`. If `From` is out of bounds, reproduces the 
   // originally specified bool sequence type.
   template<typename Sequence, size_t From>
   using clear_all_from = impl::_clear_all_from<Sequence, From>::type;
}