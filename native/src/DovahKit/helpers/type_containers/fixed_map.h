#pragma once
#include <array>
#include <concepts>
#include <type_traits>

namespace cobb::type_containers {
   template<typename T, auto V>
   struct fixed_map_entry {
      fixed_map_entry() = delete;
      ~fixed_map_entry() = delete;

      using value_type = std::decay_t<decltype(V)>;

      using key_type = T;
      static constexpr const value_type value = V;
   };

   namespace impl {
      template<typename T>
      concept is_fixed_map_entry = requires {
         typename T::key_type;
         typename T::value_type;
         { T::value } -> std::same_as<const typename T::value_type&>;
         requires std::is_same_v<
            T,
            fixed_map_entry<typename T::key_type, T::value>
         >;
      };

      namespace _fixed_map {
         #pragma region struct all_keys_unique<typename...>
            template<typename... Entries>
            struct all_keys_unique;

            template<typename... Entries> requires (sizeof...(Entries) <= 1)
            struct all_keys_unique<Entries...> {
               static constexpr const bool value = true;
            };

            template<typename First, typename... Others> requires (sizeof...(Others) > 0)
            struct all_keys_unique<First, Others...> {
               static constexpr const bool value = (!std::is_same_v<typename First::key_type, typename Others::key_type> && ...);
            };
         #pragma endregion

         #pragma region struct all_same_value_type<typename...>
            template<typename... Entries>
            struct all_same_value_type;

            template<typename... Entries> requires (sizeof...(Entries) <= 1)
            struct all_same_value_type<Entries...> {
               static constexpr const bool value = true;
            };
            
            template<typename T, typename... Others> requires (sizeof...(Others) > 0)
            struct all_same_value_type<T, Others...> {
               static constexpr const bool value = (std::is_same_v<typename T::value_type, typename Others::value_type> && ...);
            };
         #pragma endregion

         #pragma region struct common_value_type<typename...>
            template<typename... Entries>
            struct common_value_type;
            
            template<typename T, typename... Others>
            struct common_value_type<T, Others...> {
               using value_type = typename T::value_type;
            };
         #pragma endregion
      }
   }

   //
   // Represents a map of types to values. All values must be of the same type.
   // "Instantiate" a map via a using declaration, not by constructing an instance.
   //
   template<impl::is_fixed_map_entry... Entries> requires (impl::_fixed_map::all_keys_unique<Entries...>::value && impl::_fixed_map::all_same_value_type<Entries...>::value)
   class fixed_map {
      public:
         using value_type = typename impl::_fixed_map::common_value_type<Entries...>::value_type;

         static constexpr const size_t count = sizeof...(Entries);

         template<typename Key>
         static constexpr const bool has_key = (std::is_same_v<typename Entries::key_type, Key> || ...);

         template<template<typename...> class UnpackInto>
         using unpack_types_into = UnpackInto<typename Entries::key_type...>;

         template<template<typename...> class UnpackInto, template<typename> class Wrapper>
         using unpack_wrapped_types_into = UnpackInto<Wrapper<typename Entries::key_type>...>;

      protected:
         static constexpr const std::array<value_type, sizeof...(Entries)> values = {
            (Entries::value)...
         };

         template<typename Key> requires has_key<Key>
         static constexpr const size_t index_of_key = []() -> size_t {
            size_t i = 0;
            size_t n = 0;
            ((std::is_same_v<Key, typename Entries::key_type> ? (n = i++) : i++), ...);
            return n;
         }();

      public:
         template<typename Key> requires has_key<Key>
         static constexpr const value_type value_of = values[index_of_key<Key>];

         // Execute a functor templated on whatever type is mapped to the input value. Note that 
         // your functor must be valid for all types in the map (i.e. it must be able to compile 
         // without errors for any of them).
         template<typename Functor, typename... Args>
         static constexpr bool for_value(value_type value, Functor&& functor, Args&&... args) {
            bool any_executed = (
               (
                  value == Entries::value ?
                     (functor.template operator()<typename Entries::key_type>(std::forward<Args>(args)...), true)
                  :
                     false
               ) ||
               ...
            ) || false;
            return any_executed;
         }

         template<typename Functor, typename... Args>
         static constexpr void for_each_pair(Functor&& functor, Args&&... args) {
            (functor.template operator()<typename Entries::key_type, Entries::value>(std::forward<Args>(args)...) , ...);
         }

         template<typename Functor, typename... Args>
         static constexpr bool for_each_pair_until_true(Functor&& functor, Args&&... args) {
            return (functor.template operator()<typename Entries::key_type, Entries::value>(std::forward<Args>(args)...) || ...) || false;
         }

         template<typename Functor, typename... Args>
         static constexpr bool for_each_pair_until_false(Functor&& functor, Args&&... args) {
            return (functor.template operator()<typename Entries::key_type, Entries::value>(std::forward<Args>(args)...) && ...) && true;
         }
   };
}