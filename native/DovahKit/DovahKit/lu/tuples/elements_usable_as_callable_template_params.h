#pragma once
#include <concepts>
#include <tuple>
#include "../type_aliases/tuple_like_to_tuple_type.h"
#include "../type_traits/is_std_tuple.h"

namespace lu::tuples {
   template<typename TupleOrTupleLike, template<typename> typename Callable, typename... Args>
   struct elements_usable_as_callable_template_params;
   
   template<template<typename> typename Callable, typename TupleLike, typename... Args> requires (!is_std_tuple_v<TupleLike>)
   struct elements_usable_as_callable_template_params<TupleLike, Callable, Args...>
      : public elements_usable_as_callable_template_params<tuple_like_to_tuple_type<TupleLike>, Callable, Args...>
   {};

   template<template<typename> typename Callable, typename... Args, typename... Elements>
   struct elements_usable_as_callable_template_params<std::tuple<Elements...>, Callable, Args...> {
      template<typename T>
      static constexpr const bool invocable_with_element = std::is_invocable_v<Callable<T>, Args...>;
      template<typename Return, typename T>
      static constexpr const bool invocable_with_element_r = std::is_invocable_r_v<Return, Callable<T>, Args...>;

      static constexpr const size_t count_usable = ((invocable_with_element<Elements> ? (size_t)1 : (size_t)0) + ...);
      static constexpr const bool   all_usable   = count_usable == sizeof...(Elements);
      static constexpr const bool   none_usable  = count_usable == 0 && sizeof...(Elements) > 0;

      template<typename Return>
      static constexpr const size_t count_usable_r = ((invocable_with_element_r<Return, Elements> ? (size_t)1 : (size_t)0) + ...);
      template<typename Return>
      static constexpr const bool all_usable_r = count_usable_r<Return> == sizeof...(Elements);
      template<typename Return>
      static constexpr const bool none_usable_r = count_usable_r<Return> == 0 && sizeof...(Elements) > 0;
   };
}