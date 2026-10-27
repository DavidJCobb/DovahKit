#pragma once
#include <tuple>
#include "../type_aliases/tuple_like_to_tuple_type.h"
#include "./elements_usable_as_lambda_template_params.h"

namespace lu::tuples {
   namespace impl {
      template<typename TupleLike, auto Predicate>
      struct _all_types_match_predicate;

      template<auto Predicate, typename... Types>
      struct _all_types_match_predicate<std::tuple<Types...>, Predicate> {
         static constexpr const bool value = (Predicate.template operator()<Types>() && ...);
      };

      template<auto Predicate>
      struct _all_types_match_predicate<std::tuple<>, Predicate> {
         static constexpr const bool value = true;
      };
   }

   template<typename TupleLike, auto Predicate>
   concept all_types_match_predicate = requires {
      requires elements_usable_as_lambda_template_params<TupleLike, decltype(Predicate)>::template all_usable_r<bool>;
      requires impl::_all_types_match_predicate<tuple_like_to_tuple_type<TupleLike>, Predicate>::value;
   };
}