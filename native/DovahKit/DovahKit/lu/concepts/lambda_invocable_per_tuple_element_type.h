#pragma once
#include <tuple>
#include <utility>
#include "./supports_std_tuple_element.h"
#include "./supports_std_tuple_size.h"

/*
   Given:

    - Lambda type L
    - Tuple-like type T with element types E
    - Argument types A

   Tests whether the non-static member function `L::template operator()<E>` can be invoked 
   with the given argument types, for each E.
*/

namespace lu::concepts {
   namespace impl::_lambda_invocable_per_tuple_element_type {
      #pragma region lambda_invocable_per_tuple_element_type
         // Helper concept; needed as of 04/26/2026 because IntelliSense/EDG chokes if we 
         // try to define this as a `static constexpr const bool foo = requires` within a 
         // templated struct such as `is_invocable_for_tp`, below. EDG breaks when seeing 
         // a `requires` expression inside a nested template, when the expression depends 
         // on an outer template's parameters.
         template<typename Lambda, typename Element, typename... Args>
         concept invocable_for_tp = requires(Lambda&& f, Args&&... args) {
            { f.template operator()<Element>(std::forward<Args>(args)...) };
         };

         // Baseline implementation for tuple-like types that are not `std::tuple`. Use a 
         // specialization of `std::index_sequence` to loop over each tuple element type.
         template<typename Lambda, typename TupleLike, typename... Args>
         struct is_invocable_for_tp {
            template<typename IS>
            struct exec;
            template<auto... I> requires (sizeof...(I) > 0)
            struct exec<std::index_sequence<I...>> {
               static constexpr const bool value = (invocable_for_tp<Lambda, std::tuple_element_t<I, TupleLike>, Args...> && ...);
            };

            static constexpr const bool value = exec<std::make_index_sequence<std::tuple_size_v<TupleLike>>>::value;
         };

         // Fast-path specialization for `std::tuple` specializations:
         template<typename Lambda, typename... Elements, typename... Args>
         struct is_invocable_for_tp<Lambda, std::tuple<Elements...>, Args...> {
            static constexpr const bool value = (invocable_for_tp<Lambda, Elements, Args...> && ...);
         };
      #pragma endregion
      #pragma region lambda_invocable_per_tuple_element_type_with_result
         // Helper concept; needed as of 04/26/2026 because IntelliSense/EDG chokes if we 
         // try to define this as a `static constexpr const bool foo = requires` within a 
         // templated struct such as `is_invocable_for_tp_r`, below. EDG breaks when seeing 
         // a `requires` expression inside a nested template, when the expression depends 
         // on an outer template's parameters.
         template<typename Lambda, typename Element, typename Result, typename... Args>
         concept invocable_for_tp_r = requires(Lambda&& f, Args&&... args) {
            { f.template operator()<Element>(std::forward<Args>(args)...) } -> std::same_as<Result>;
         };

         // Baseline implementation for tuple-like types that are not `std::tuple`. Use a 
         // specialization of `std::index_sequence` to loop over each tuple element type.
         template<typename Lambda, typename TupleLike, typename Result, typename... Args>
         struct is_invocable_for_tp_r {
            template<typename IS>
            struct exec;
            template<auto... I>
            struct exec<std::index_sequence<I...>> {
               static constexpr const bool value = (invocable_for_tp_r<Lambda, std::tuple_element_t<I, TupleLike>, Result, Args...> && ...);
            };

            static constexpr const bool value = exec<std::make_index_sequence<std::tuple_size_v<TupleLike>>>::value;
         };

         // Fast-path specialization for `std::tuple` specializations:
         template<typename Lambda, typename Result, typename... Elements, typename... Args>
         struct is_invocable_for_tp_r<Lambda, std::tuple<Elements...>, Result, Args...> {
            static constexpr const bool value = (invocable_for_tp_r<Lambda, Elements, Result, Args...> && ...);
         };
      #pragma endregion

   }

   template<typename Functor, typename TupleLike, typename... Args>
   concept lambda_invocable_per_tuple_element_type = requires {
      requires supports_std_tuple_size<TupleLike>;
      requires (std::tuple_size_v<TupleLike> == 0 || supports_std_tuple_element<TupleLike>);
      requires impl::_lambda_invocable_per_tuple_element_type::is_invocable_for_tp<Functor, TupleLike, Args...>::value;
   };
   
   template<typename Functor, typename TupleLike, typename Result, typename... Args>
   concept lambda_invocable_per_tuple_element_type_with_result = requires {
      requires supports_std_tuple_size<TupleLike>;
      requires (std::tuple_size_v<TupleLike> == 0 || supports_std_tuple_element<TupleLike>);
      requires impl::_lambda_invocable_per_tuple_element_type::is_invocable_for_tp_r<Functor, TupleLike, Result, Args...>::value;
   };
}