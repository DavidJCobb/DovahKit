#pragma once
#include <utility>

namespace lu::utils::bool_sequences {
   namespace impl {
      template<typename Sequence>
      struct _sequence_has_one_true_run;

      template<bool... Bools>
      struct _sequence_has_one_true_run<std::integer_sequence<bool, Bools...>> {
         static constexpr const bool any_true = (Bools || ...);

         static constexpr const size_t first_true = []() {
            size_t i      = 0;
            size_t result = 0;
            (((Bools ? (result = i),false : true) && ++i) && ...);
            return result;
         }();

         static constexpr const size_t first_false_after_true = []() {
            size_t i      = 0;
            size_t result = sizeof...(Bools);
            (((!Bools && i > first_true ? (result=i),false : true) && ++i) && ...);
            return result;
         }();

         static constexpr const bool result = []() {
            size_t i      = 0;
            bool   result = true;
            (((Bools && i > first_false_after_true ? (result=false) : true) && ++i) && ...);
            return result;
         }();
      };
   }

   // True if the template parameter is an `std::integer_sequence<bool, ...>` and it contains exactly 
   // one series of contiguous `true` values. False if all values are `false`, or if there are `true` 
   // values that are not consecutive with one another.
   template<typename Sequence>
   constexpr const bool sequence_has_one_true_run = impl::_sequence_has_one_true_run<Sequence>::any_true && impl::_sequence_has_one_true_run<Sequence>::result;
}
