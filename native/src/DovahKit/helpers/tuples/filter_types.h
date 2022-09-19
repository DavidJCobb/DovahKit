#pragma once
#include <tuple>
#include <type_traits>
#include "./concat.h"
#include "./prepend.h"

namespace cobb::tuples {
   namespace impl::_filter_types {
      template<auto Predicate, typename...> struct variadic_filter;
      template<auto Predicate, typename... Types> using variadic_filter_t = typename variadic_filter<Predicate, Types...>::type;
      
      template<auto Predicate> struct variadic_filter<Predicate> {
         using type = std::tuple<>;
      };
      
      template<auto Predicate, typename First, typename... Next>
      struct variadic_filter<Predicate, First, Next...> {
         using type = std::conditional_t<
            (Predicate.template operator()<First>()),
            prepend<First, variadic_filter_t<Predicate, Next...>>,
            variadic_filter_t<Predicate, Next...>
         >;
      };

      template<bool> struct retain {
         template<typename Item> using type = std::tuple<Item>;
      };
      template<> struct retain<false> {
         template<typename Item> using type = std::tuple<>;
      };

      // ---

      template<auto Predicate, typename> struct result;
      template<auto Predicate, typename... Types> struct result<Predicate, std::tuple<Types...>> {
         using type = decltype(
            std::tuple_cat(
               std::declval<
                  typename retain<
                     Predicate.template operator()<Types>()
                  >::template type<Types>
               >()...
            )
         );
      };
   }

   template<typename Tuple, auto Predicate> using filter_types = typename impl::_filter_types::result<Predicate, Tuple>::type;
}