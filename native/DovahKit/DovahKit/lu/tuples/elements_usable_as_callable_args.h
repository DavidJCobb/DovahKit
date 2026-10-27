#pragma once
#include <concepts>
#include <tuple>
#include "../type_aliases/tuple_like_to_tuple_type.h"
#include "../type_traits/is_std_tuple.h"

namespace lu::tuples {
   template<typename TupleOrTupleLike, typename Callable>
   struct elements_usable_as_callable_args;
   
   template<typename Callable, typename TupleLike> requires (!is_std_tuple_v<TupleLike>)
   struct elements_usable_as_callable_args<TupleLike, Callable> : public elements_usable_as_callable_args<tuple_like_to_tuple_type<TupleLike>, Callable> {};

   template<typename Callable, typename... Elements>
   struct elements_usable_as_callable_args<std::tuple<Elements...>, Callable> {
      template<typename T>
      static constexpr const bool invocable_with_element = requires(Callable&& f, T&& v) {
         { f(v) };
      };
      template<typename Return, typename T>
      static constexpr const bool invocable_with_element_r = requires(Callable&& f, T&& v) {
         { f(v) } -> std::same_as<Return>;
      };

      static constexpr const size_t count_usable = ((size_t)(invocable_with_element<Elements> ? 1 : 0) + ...);
      static constexpr const bool   all_usable   = count_usable == sizeof...(Elements);
      static constexpr const bool   none_usable  = count_usable == 0;

      template<typename Return>
      static constexpr const size_t count_usable_r = ((size_t)(invocable_with_element_r<Return, Elements> ? 1 : 0) + ...);
      template<typename Return>
      static constexpr const bool all_usable_r = count_usable_r<Return> == sizeof...(Elements);
      template<typename Return>
      static constexpr const bool none_usable_r = count_usable_r<Return> == 0;
   };
}