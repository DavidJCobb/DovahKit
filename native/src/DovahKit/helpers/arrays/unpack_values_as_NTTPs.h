#pragma once
#include <array>
#include <utility>

//
// Intended usage:
// 
//    using unpacked = std::unpack_values_as_nttp<std::array{0, 1, 2}, PrependedA, PrependedB>::into<Template>;
//    static_assert(std::is_same_v<unpacked, Template<PrependedA, PrependedB, 0, 1, 2>>);
// 
// Can specify an arbitrary number of prepended types (including zero), but there 
// is a maximum (an arbitrary one, limited by how much I want to copy and paste).
//

namespace cobb::arrays {
   namespace impl::unpack_values_as_nttp {
      template<auto Array>
      struct a {
         using array_type = std::decay_t<decltype(Array)>;
         using value_type = typename array_type::value_type;

         template<typename Sequence> struct b_impl;
         template<size_t... Indices> struct b_impl<std::index_sequence<Indices...>> {
            template<typename... Prepend>
            struct c;

            template<>
            struct c<> {
               template<template<value_type...> typename Target>
               using into = Target<Array[Indices]...>;
            };
            
            template<typename T1>
            struct c<T1> {
               template<template<typename, value_type...> typename Target>
               using into = Target<T1, Array[Indices]...>;
            };
            
            template<typename T1, typename T2>
            struct c<T1, T2> {
               template<template<typename, typename, value_type...> typename Target>
               using into = Target<T1, T2, Array[Indices]...>;
            };
            
            template<typename T1, typename T2, typename T3>
            struct c<T1, T2, T3> {
               template<template<typename, typename, typename, value_type...> typename Target>
               using into = Target<T1, T2, T3, Array[Indices]...>;
            };

            template<typename T1, typename T2, typename T3, typename T4>
            struct c<T1, T2, T3, T4> {
               template<template<typename, typename, typename, typename, value_type...> typename Target>
               using into = Target<T1, T2, T3, T4, Array[Indices]...>;
            };

            template<typename T1, typename T2, typename T3, typename T4, typename T5>
            struct c<T1, T2, T3, T4, T5> {
               template<template<typename, typename, typename, typename, typename, value_type...> typename Target>
               using into = Target<T1, T2, T3, T4, T5, Array[Indices]...>;
            };
         };

         using b = b_impl<std::make_index_sequence<std::tuple_size_v<array_type>>>;
      };
   }

   template<
      auto Array,
      typename... Prepend
   > using unpack_values_as_nttp = impl::unpack_values_as_nttp::template a<Array>::b::template c<Prepend...>;
}