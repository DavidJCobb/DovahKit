#pragma once
#include <utility>

namespace lu::utils {
   namespace impl {
      template<typename IS, std::size_t Start, std::size_t Size>
      struct index_sequence_to_bool_run_impl;

      template<std::size_t Start, std::size_t Size, std::size_t... I>
         requires (Start + Size <= sizeof...(I))
      struct index_sequence_to_bool_run_impl<std::index_sequence<I...>, Start, Size> {
         private:
            static constexpr const auto end = Start + Size;
         public:
            using type = std::integer_sequence<bool, (I >= Start && I < end)...>;
      };
   }

   // Given a `std::index_sequence`, a start index, and a size, produces a 
   // `std::integer_sequence<bool, ...>` of equivalent size wherein all of 
   // the NTTPs in the range [Start, Start + Size) are `true`, and the rest 
   // are `false`.
   template<typename IS, std::size_t Start, std::size_t Size>
   using index_sequence_to_bool_run = impl::index_sequence_to_bool_run_impl<IS, Start, Size>::type;
}