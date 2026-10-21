#pragma once
#include <concepts>
#include <tuple>
#include <type_traits>
#include "../foreach_lambda_result.h"
#include "../tuples/any_types_match_predicate.h"
#include "../tuples/contains_type.h"
#include "../tuples/elements_usable_as_lambda_template_params.h"
#include "../tuples/filter_types_by_predicate.h"
#include "../tuples/for_each_type.h"
#include "../tuples/indices_of_types_matching_predicate.h"
#include "../tuples/map_types.h"
#include "../tuples/unpack_types_into_template.h"

namespace lu::type_containers {
   template<typename... Types>
   class type_array {
      public:
         using as_tuple = std::tuple<Types...>;
         
         static constexpr const size_t index_of_none = (size_t)-1;

         type_array() = delete;
         type_array(const type_array&) = delete;
         type_array(type_array&&) = delete;
         ~type_array() = delete;

      public:
         template<size_t N> requires (N < sizeof...(Types))
         using nth_type = typename std::tuple_element<N, as_tuple>::type;

         static constexpr size_t count = sizeof...(Types);
         static consteval size_t size() noexcept { return count; }

         template<typename T>
         static constexpr bool contains_type = tuples::contains_type<as_tuple, T>;

         template<typename T>
         static constexpr size_t index_of_type = []() {
            size_t result = index_of_none;
            size_t i      = 0;
            (((std::is_same_v<T, Types> ? (result = i) : 0), ++i), ...);
            return result;
         }();

         template<auto Predicate>
         static constexpr bool contains_matching_type = tuples::any_types_match_predicate<as_tuple, Predicate>;

         template<auto Predicate>
         static constexpr size_t index_of_matching_type = []() {
            constexpr const auto matches = tuples::indices_of_types_matching_predicate<as_tuple, Predicate>;
            if (matches.empty())
               return index_of_none;
            else
               return matches[0];
         }();

         template<auto Predicate> requires (index_of_matching_type<Predicate> != index_of_none)
         using get_matching_type = nth_type<index_of_matching_type<Predicate>>;

         #pragma region for_each
            template<auto Lambda, typename... Args>
            static constexpr void for_each(Args&&... args) {
               tuples::for_each_type<as_tuple>([&args...]<typename Current>() {
                  Lambda.template operator()<Current>(std::forward<Args>(args)...);
               });
            }

            template<typename Lambda, typename... Args>
            static constexpr void for_each(Lambda&& lambda, Args&&... args) {
               tuples::for_each_type<as_tuple>([&lambda, &args...]<typename Current>() {
                  std::forward<Lambda>(lambda).template operator()<Current>(std::forward<Args>(args)...);
               });
            }
         #pragma endregion

         template<typename Result, auto Lambda>
            requires tuples::elements_usable_as_lambda_template_params<as_tuple, std::decay_t<decltype(Lambda)>>::template all_usable_r<Result, const Result&>
         static constexpr Result reduce(Result prev = {}) {
            return ((prev = Lambda.template operator()<Types>(prev)), ...);
         }

         /*//
         template<auto Predicate>
         using filter_types = tuples::unpack_types_into_template<typename tuples::filter_types_by_predicate<as_tuple, Predicate>, ::lu::type_containers::type_array>;
         //*/
         template<template<typename> typename Transform>
         using map_types = tuples::unpack_types_into_template<tuples::map_types<as_tuple, Transform>, ::lu::type_containers::type_array>;
   };
}
