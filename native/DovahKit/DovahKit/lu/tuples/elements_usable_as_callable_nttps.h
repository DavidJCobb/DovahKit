#pragma once
#include <concepts>
#include <tuple>
#include <utility> // index_sequence and friends

namespace lu::tuples {
   template<const auto& TupleOrTupleLike, template<auto...> typename Callable, typename... Args>
   struct elements_usable_as_callable_nttps {
      using tuple_type = std::decay_t<decltype(TupleOrTupleLike)>;

      static constexpr const size_t tuple_size = std::tuple_size_v<tuple_type>;

      template<size_t Index>
      using nth_callable = Callable<std::get<Index>(TupleOrTupleLike)>;

      template<size_t Index>
      static constexpr const bool invocable_with_element = std::is_invocable_v<nth_callable<Index>, Args...>;
      template<typename Return, size_t Index>
      static constexpr const bool invocable_with_element_r = std::is_invocable_r_v<Return, nth_callable<Index>, Args...>;

      template<typename IS = std::make_index_sequence<tuple_size>>
      struct helper;
      template<auto... I>
      struct helper<std::index_sequence<I...>> {
         static constexpr const size_t count_usable = ((size_t)(invocable_with_element<I> ? 1 : 0) + ...);

         template<typename Return>
         static constexpr const size_t count_usable_r = ((size_t)(invocable_with_element_r<Return, I> ? 1 : 0) + ...);
      };

      static constexpr const size_t count_usable = helper<>::count_usable;
      static constexpr const size_t all_usable   = count_usable == tuple_size;
      static constexpr const size_t none_usable  = count_usable == 0 && tuple_size > 0;

      template<typename Return>
      static constexpr const size_t count_usable_r = helper<>::template count_usable_r<Return>;
      template<typename Return>
      static constexpr const bool all_usable_r = count_usable_r<Return> == tuple_size;
      template<typename Return>
      static constexpr const bool none_usable_r = count_usable_r<Return> == 0 && tuple_size > 0;
   };
}