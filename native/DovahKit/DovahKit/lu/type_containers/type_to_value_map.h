#pragma once
#include <array>
#include <concepts>
#include <tuple>
#include <type_traits>
#include "../concepts/all_same_as.h"
#include "../concepts/lambda_invocable_per_tuple_element_type.h"
#include "../foreach_lambda_result.h"

namespace lu::type_containers {
   template<typename T, auto V>
   class type_to_value_map_pair {
      public:
         type_to_value_map_pair() = delete;
         ~type_to_value_map_pair() = delete;
      public:
         using value_type = std::decay_t<decltype(V)>;

         using key_type = T;
         static constexpr const value_type value = V;
   };

   namespace impl {
      template<typename T>
      concept is_type_to_value_map_pair = requires {
         typename T::key_type;
         typename T::value_type;
         { T::value } -> std::same_as<const typename T::value_type&>;
         requires std::is_same_v<
            T,
            type_to_value_map_pair<typename T::key_type, T::value>
         >;
      };

      namespace _type_to_value_map {
         #pragma region all_keys_unique
            template<typename... Pairs>
            struct all_keys_unique_test;
            //
            template<>
            struct all_keys_unique_test<> : public std::true_type {};
            //
            template<typename First, typename... Others>
            struct all_keys_unique_test<First, Others...> {
               static constexpr const bool value = (!std::is_same_v<typename First::key_type, typename Others::key_type> && ...);
            };

            template<typename... Pairs>
            constexpr const bool all_keys_unique = all_keys_unique_test<Pairs...>::value;
         #pragma endregion

         template<typename FirstPair, typename... Pairs>
         constexpr const bool all_same_value_type = []() {
            if constexpr (sizeof...(Pairs) >= 1) {
               return (std::is_same_v<typename FirstPair::value_type, typename Pairs::value_type> && ...);
            } else {
               return true;
            }
         }();

         #pragma region value_type_of_pairs
            template<typename... Pairs>
            struct value_type_of_pairs_impl;
            //
            template<typename FirstPair, typename... Ignore>
            struct value_type_of_pairs_impl<FirstPair, Ignore...> {
               using type = FirstPair::value_type;
            };

            template<typename... Pairs>
            using value_type_of_pairs = value_type_of_pairs_impl<Pairs...>::type;
         #pragma endregion

         template<typename Callable, typename Pair, typename... Args>
         concept callable_invocable_for_pair = requires(Callable&& callable , Args&&... args) {
            typename Pair::key_type;
            { Pair::value };
            { callable.template operator()<typename Pair::key_type, Pair::value>(std::forward<Args>(args)...) };
         };

         template<typename Callable, typename Pair, typename... Args>
            requires callable_invocable_for_pair<Callable, Pair, Args...>
         using callable_invoke_result_for_pair = std::invoke_result_t<
            decltype(&Callable::template operator()<typename Pair::key_type, Pair::value>),
            Callable&, // `this` pointer for operator
            Args...
         >;

         template<typename Callable, typename Key, typename... Args>
         using callable_invoke_result_for_key = std::invoke_result_t<
            decltype(&Callable::template operator()< Key >),
            Callable&, // `this` pointer for operator
            Args...
         >;
      }
   }

   template<typename... Pairs>
      requires (
         sizeof...(Pairs) > 0 && 
         (impl::is_type_to_value_map_pair<Pairs> && ...) &&
         impl::_type_to_value_map::all_keys_unique<Pairs...> &&
         impl::_type_to_value_map::all_same_value_type<Pairs...>
      )
   class type_to_value_map {
      public:
         using value_type = impl::_type_to_value_map::value_type_of_pairs<Pairs...>;
         static constexpr const size_t count = sizeof...(Pairs);
         static constexpr const size_t index_of_none = (size_t)-1;

      protected:
         static constexpr const std::array<value_type, count> all_values = {
            (Pairs::value)...
         };

         using pair_tuple = std::tuple<Pairs...>;
         
      public:
         template<size_t N>
            requires (N < sizeof...(Pairs)) // [10/21/2026] using `count` here makes EDG/IntelliSense choke for some reason
         using nth_pair = std::tuple_element_t<N, pair_tuple>;

         #pragma region Key-related accessors
            template<typename Key>
            static constexpr const bool has_key = (std::is_same_v<typename Pairs::key_type, Key> || ...);

            template<typename Key>
            static constexpr const size_t index_of_key = []() -> size_t {
               size_t i = 0;
               size_t n = index_of_none;
               ((std::is_same_v<Key, typename Pairs::key_type> ? (n = i++) : i++), ...);
               return n;
            }();

            template<size_t N>
               requires (N < sizeof...(Pairs)) // [10/21/2026] using `count` here makes EDG/IntelliSense choke for some reason
            using nth_key_type = nth_pair<N>::key_type;

            template<typename Key>
               requires has_key<Key>
            static constexpr const value_type value_of = all_values[index_of_key<Key>];
         #pragma endregion

         #pragma region Value-related accessors
            template<value_type Value>
            static constexpr const size_t index_of_value = []() -> size_t {
               for (size_t i = 0; i < all_values.size(); ++i)
                  if (all_values[i] == Value)
                     return i;
               return index_of_none;
            }();
            
            template<value_type Value>
            static constexpr const size_t has_value = index_of_value<Value> != index_of_none;
         
            template<auto Value> requires (index_of_value<Value> < count)
            using key_of = nth_key_type<index_of_value<Value>>;
         #pragma endregion

         // Execute a callable's templated `operator()`, passing as a template parameter whatever 
         // key type matches the value you've passed in as an argument.
         template<typename Callable, typename... Args>
            // The callable must be invocable for all key types (i.e. it must be able to compile 
            // without errors for any of them).
            requires concepts::lambda_invocable_per_tuple_element_type<Callable, std::tuple<typename Pairs::key_type...>, Args...>
         static constexpr bool invoke_for_type_by_value(value_type value, Callable&& callable, Args&&... args) {
            bool any_executed = (
               (
                  (value == Pairs::value) ?
                     (callable.template operator()<typename Pairs::key_type>(std::forward<Args>(args)...), true)
                  :
                     false
               ) ||
               ...
            ) || false;
            return any_executed;
         }

         // Given a callable whose `operator()` takes as template parameters a type and an NTTP of 
         // type `value_type`, execute that callable for every pair in the map. If the callable 
         // returns a value of type `foreach_lambda_result`, then stop execution if any call returns 
         // the value `foreach_lambda_result::stop`.
         template<typename Callable, typename... Args>
            requires (
               // The callable must be invocable for all key/value pairs in the map.
               (impl::_type_to_value_map::callable_invocable_for_pair<Callable, Pairs, Args...> && ...)
               // The callable must have the same return type for all key/value pairs in the map.
            && (concepts::all_same_as<
                  impl::_type_to_value_map::callable_invoke_result_for_pair<Callable, nth_pair<0>, Args...>,
                  impl::_type_to_value_map::callable_invoke_result_for_pair<Callable, Pairs, Args...>...
               >)
            )
         static constexpr void for_each_pair(Callable&& callable, Args&&... args) {
            if constexpr (std::is_same_v<impl::_type_to_value_map::callable_invoke_result_for_pair<Callable, nth_pair<0>, Args...>, foreach_lambda_result>) {
               (
                  (callable.template operator()<typename Pairs::key_type, Pairs::value>(std::forward<Args>(args)) != foreach_lambda_result::stop)
                  || ...
               );
            } else {
               (
                  callable.template operator()<typename Pairs::key_type, Pairs::value>(std::forward<Args>(args)...)
                  , ...
               );
            }
         }

         // Given a callable whose `operator()` takes as template parameters a type, execute that 
         // callable as templated on every key in the map. If the callable returns a value of type 
         // `foreach_lambda_result`, then stop execution if any call returns the value `stop`.
         template<typename Callable, typename... Args>
            requires (
               // The callable must be invocable for all keys in the map.
               concepts::lambda_invocable_per_tuple_element_type<Callable, std::tuple<typename Pairs::key_type...>, Args...>
               // The callable must have the same return type for all key/value pairs in the map.
            && (concepts::all_same_as<
                  impl::_type_to_value_map::callable_invoke_result_for_key<Callable, nth_key_type<0>, Args...>,
                  impl::_type_to_value_map::callable_invoke_result_for_key<Callable, typename Pairs::key_type, Args...>...
               >)
            )
         static constexpr void for_each_key(Callable&& callable, Args&&... args) {
            if constexpr (std::is_same_v<impl::_type_to_value_map::callable_invoke_result_for_key<Callable, nth_key_type<0>, Args...>, foreach_lambda_result>) {
               (
                  (callable.template operator()<typename Pairs::key_type>(std::forward<Args>(args)) != foreach_lambda_result::stop)
                  || ...
               );
            } else {
               (
                  callable.template operator()<typename Pairs::key_type>(std::forward<Args>(args)...)
                  , ...
               );
            }
         }

         template<template<typename...> typename Template>
         using unpack_types_into = Template<typename Pairs::key_type...>;
   };
}