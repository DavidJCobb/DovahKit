#pragma once
#include <utility>
#include "./sequence_has_one_true_run.h"

namespace lu::utils::bool_sequences {
   // Given a `std::integer_sequence<bool, ...>` type which contains exactly one run 
   // of consecutive `true` values, locates that run and reports the start and end 
   // indices, and the length.
   template<typename Sequence>
      requires sequence_has_one_true_run<Sequence>
   struct true_run_extents_of;

   template<bool... Bools>
   struct true_run_extents_of<std::integer_sequence<bool, Bools...>> {
      static constexpr const size_t start  = []() {
         size_t i      = 0;
         size_t result = 0;
         ((Bools ? (result = i),false : ++i,true) && ...);
         return result;
      }();
      static constexpr const size_t length = ((size_t)Bools + ...);
      static constexpr const size_t end    = start + length;
   };
}