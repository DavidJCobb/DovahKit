#pragma once
#include <tuple>

namespace cobb::tuples {
   namespace impl {
      template<typename Tuple, auto Predicate>
      struct _contains_type_matching_functor;

      template<auto Predicate, typename... Types> requires requires {
         { (Predicate.template operator()<Types>() || ...) } -> std::same_as<bool>;
      }
      struct _contains_type_matching_functor<std::tuple<Types...>, Predicate> {
         static constexpr const bool value = (Predicate.template operator()<Types>() || ...);
      };

      template<auto Predicate>
      struct _contains_type_matching_functor<std::tuple<>, Predicate> {
         static constexpr const bool value = false;
      };
   }

   template<typename Tuple, auto Predicate>
   constexpr bool contains_type_matching_functor = impl::_contains_type_matching_functor<Tuple, Predicate>::value;
}