#pragma once
#include <array>
#include <type_traits>
#include <utility>
#include "helpers/polyfills/msvc/const_reference_nttp_decltype.h"

namespace cobb::arrays {
   template<typename Functor, typename ValueType, size_t Size>
      requires std::is_invocable<Functor, std::add_const_t<std::add_lvalue_reference_t<ValueType>>>::value
   consteval auto map(const std::array<ValueType, Size>& src, Functor&& mutator) {
      using ResultType = std::invoke_result_t<Functor, ValueType>;

      std::array<ResultType, Size> dst = {};
      for (size_t i = 0; i < Size; ++i) {
         dst[i] = mutator(src[i]);
      }
      return dst;
   }

   // Variant with index argument
   template<typename Functor, typename ValueType, size_t Size>
      requires std::is_invocable<Functor, std::add_const_t<std::add_lvalue_reference_t<ValueType>>, size_t>::value
   consteval auto map(const std::array<ValueType, Size>& src, Functor&& mutator) {
      using ResultType = std::invoke_result_t<Functor, ValueType, size_t>;

      std::array<ResultType, Size> dst = {};
      for (size_t i = 0; i < Size; ++i) {
         dst[i] = mutator(src[i], i);
      }
      return dst;
   }

   namespace impl::_map_tp {
      template<const auto& Array, typename Functor> struct types {
         using source_type = std::decay_t<cobb::polyfills::msvc::nttp_decltype<Array>>;
         using value_type  = typename source_type::value_type;
         using result_type = std::invoke_result_t<
            decltype(&std::remove_cvref_t<Functor>::template operator()<Array, 0>),
            Functor
         >;
      };
   }

   // This version allows the array to be passed as a template parameter. There are certain 
   // cases where this is useful, e.g. specializing some other template on the value of the 
   // array elements.
   // 
   // The functor receives as template parameters a const reference to the array, and the 
   // current index; the functor receives no arguments. The use of two template parameters 
   // (instead of just passing the array element directly) works around pre-P1907 requirements 
   // for non-type template parameters (wherein C++ forbade "subobjects" from being NTTPs, 
   // i.e. you could only pass references to "top-level objects" and not to their elements). 
   // P1907 was accepted into C++20, and MSVC is listed online as supporting it; however, 
   // while working on this function, I ran into IntelliSense issues which imply inconsistent 
   // support (in Visual Studio 17.3.5). Easiest to just do things the pre-P1907 way, then.
   //
   template<const auto& Array, typename Functor>
   consteval auto map_tp(Functor&& mutator) {
      using types = impl::_map_tp::types<Array, Functor>;
      constexpr const std::size_t Size = std::tuple_size_v<typename types::source_type>;

      constexpr std::size_t BatchSize = 20;
      constexpr std::size_t count_not_batched          = Size == BatchSize ? 0 : Size % BatchSize;
      constexpr std::size_t count_skipped_via_batching = Size - count_not_batched;

      std::array<typename types::result_type, Size> dst = {};
      std::size_t i = 0;
      if constexpr (Size >= BatchSize) {
         //
         // I hear compilers don't especially like huge numbers of template parameters, 
         // and the slowdowns I saw in IntelliSense while trying to get this to work 
         // certainly seem to support the idea.
         // 
         // We can do our work in batches to hopefully alleviate the strain.
         //
         [&dst, &i, &mutator] <std::size_t... Indices>(std::index_sequence<Indices...>) {
            (
               (
                  dst[i++] = mutator.template operator()<Array, Indices * BatchSize + 0>(),
                  dst[i++] = mutator.template operator()<Array, Indices * BatchSize + 1>(),
                  dst[i++] = mutator.template operator()<Array, Indices * BatchSize + 2>(),
                  dst[i++] = mutator.template operator()<Array, Indices * BatchSize + 3>(),
                  dst[i++] = mutator.template operator()<Array, Indices * BatchSize + 4>(),
                  dst[i++] = mutator.template operator()<Array, Indices * BatchSize + 5>(),
                  dst[i++] = mutator.template operator()<Array, Indices * BatchSize + 6>(),
                  dst[i++] = mutator.template operator()<Array, Indices * BatchSize + 7>(),
                  dst[i++] = mutator.template operator()<Array, Indices * BatchSize + 8>(),
                  dst[i++] = mutator.template operator()<Array, Indices * BatchSize + 9>(),
                  dst[i++] = mutator.template operator()<Array, Indices * BatchSize + 10>(),
                  dst[i++] = mutator.template operator()<Array, Indices * BatchSize + 11>(),
                  dst[i++] = mutator.template operator()<Array, Indices * BatchSize + 12>(),
                  dst[i++] = mutator.template operator()<Array, Indices * BatchSize + 13>(),
                  dst[i++] = mutator.template operator()<Array, Indices * BatchSize + 14>(),
                  dst[i++] = mutator.template operator()<Array, Indices * BatchSize + 15>(),
                  dst[i++] = mutator.template operator()<Array, Indices * BatchSize + 16>(),
                  dst[i++] = mutator.template operator()<Array, Indices * BatchSize + 17>(),
                  dst[i++] = mutator.template operator()<Array, Indices * BatchSize + 18>(),
                  dst[i++] = mutator.template operator()<Array, Indices * BatchSize + 19>()
               ),
               ...
            );
         }(std::make_index_sequence<Size / BatchSize>{});
      }
      [&dst, &i, &mutator]<std::size_t... Indices>(std::index_sequence<Indices...>){
         (
            (dst[i++] = mutator.template operator()<Array, Indices + count_skipped_via_batching>()),
            ...
         );
      }(std::make_index_sequence<count_not_batched>{});
      return dst;
   }
}