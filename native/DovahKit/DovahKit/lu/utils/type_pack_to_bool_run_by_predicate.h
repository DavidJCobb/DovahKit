#pragma once
#include "./index_sequence_to_bool_run.h"

namespace lu::utils {
   namespace impl {
      template<auto Predicate, size_t RunStart, typename... Types>
         requires (RunStart < sizeof...(Types))
      struct type_pack_to_bool_run_by_predicate_impl {
         private:
            static constexpr const bool first_in_run_is_truthy = []() {
               bool first  = true;
               bool result = false;
               ((first ? (result = Predicate.template operator()<Types>()),(first = false) : true) && ...);
               return result;
            }();
            
            static constexpr const size_t end = []() {
               size_t i      = 0;
               size_t result = sizeof...(Types);
               (
                  ((i <= RunStart ?
                     true
                  :
                     (Predicate.template operator()<Types>() != first_in_run_is_truthy ?
                        ((result = i),false)
                     :
                        true
                     )
                  ) && ++i)
                  && ...
               );
               return result;
            }();
            
         public:
            using sequence = utils::index_sequence_to_bool_run<
               std::index_sequence_for<Types...>,
               RunStart,
               (end - RunStart)
            >;
      };
   }

   // Given a boolean predicate, an index, and a parameter pack of types, this template 
   // tests each type from that index onward against the predicate, stopping at the first 
   // type that returns a different result from the previous. The template then produces 
   // a `std::integer_sequence<bool, ...>` of the same length as the type parameter pack, 
   // wherein the NTTPs are `true` for all indices for which the predicate produced the 
   // same result.
   //
   // That is: `if Predicate(Types[RunStart]) == true`, then the bool sequence contains 
   // `true` values starting at `RunStart` and continuing for as long as the following 
   // condition is met:
   //
   //    Predicate(Types[n]) == Predicate(Types[RunStart])
   // 
   // The `Predicate` should be a lambda with a templated call operator.
   //
   template<auto Predicate, size_t RunStart, typename... Types>
   using type_pack_to_bool_run_by_predicate = impl::type_pack_to_bool_run_by_predicate_impl<Predicate, RunStart, Types...>::sequence;
}