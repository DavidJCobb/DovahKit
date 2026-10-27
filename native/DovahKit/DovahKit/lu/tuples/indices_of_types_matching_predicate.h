#pragma once
#include <array>
#include <tuple>
#include <type_traits>
#include "../type_aliases/tuple_like_to_tuple_type.h"
#include "../type_traits/is_std_tuple.h"
#include "./elements_usable_as_lambda_template_params.h"

namespace lu::tuples {
   namespace impl::_indices_of_types_matching_predicate {
      template<typename Tuple, auto Predicate>
      struct matching_count;
      //
      template<auto Predicate, typename... Elements>
      struct matching_count<std::tuple<Elements...>, Predicate> :
         std::integral_constant<size_t, (size_t)((Predicate.template operator()<Elements>() ? 1 : 0) + ...)>
      {};

      template<typename TupleOrTupleLike, auto Predicate>
      struct result;
      //
      template<typename TupleLike, auto Predicate> requires (!is_std_tuple_v<TupleLike>)
      struct result<TupleLike, Predicate> : public result<tuple_like_to_tuple_type<TupleLike>, Predicate> {};
      //
      template<auto Predicate, typename... Elements>
      struct result<std::tuple<Elements...>, Predicate> {
         static constexpr const size_t size  = matching_count<std::tuple<Elements...>, Predicate>::value;
         static constexpr const auto   value = [](){
            std::array<size_t, size> out = {};
            size_t i = 0;
            size_t o = 0;
            (
               (
                  Predicate.template operator()<Elements>() ?
                     (out[o++] = i++)
                  :
                     i++
               ),
               ...
            );
            return out;
         }();
      };
   }

   template<typename TupleOrTupleLike, auto Predicate>
      requires (
         // Require that the tuple or tuple-like type... actually be that.
         (concepts::supports_std_tuple_size<TupleOrTupleLike>&& concepts::supports_std_tuple_element<TupleOrTupleLike>)

         // Require that `Predicate.template operator()<E>()` be a valid expression for each element 
         // type `E` in the tuple or tuple-like type, and that that expression produce a `bool`.
      && elements_usable_as_lambda_template_params<TupleOrTupleLike, decltype(Predicate)>::template all_usable_r<bool>
      )
   constexpr const auto indices_of_types_matching_predicate = impl::_indices_of_types_matching_predicate::result<TupleOrTupleLike, Predicate>::value;
}