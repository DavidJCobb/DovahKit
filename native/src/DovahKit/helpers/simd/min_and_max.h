#pragma once
#include <array>
#include <concepts>
#include <intrin.h>

namespace cobb::simd {
   namespace impl::min_and_max {
      template<template<typename, size_t> typename Container, size_t Size> concept is_bounded_float_container = requires(const Container<float, Size>& list, size_t i) {
         { list[i] } -> std::same_as<const float&>;
      };
   }

   template<template<typename, size_t> typename Container, size_t Size> requires impl::min_and_max::is_bounded_float_container<Container, Size>
   constexpr float maximum_of_list(const Container<float, Size>& list) {
      constexpr auto initial = std::numeric_limits<float>::lowest();
      //
      size_t i = 0;
      //
      auto simd = _mm_set_ps1(initial);
      for (; i + 7 < Size; i += 8) {
         auto a = _mm_loadu_ps(&list[i + 0]);
         auto b = _mm_loadu_ps(&list[i + 4]);
         simd = _mm_max_ps(simd, _mm_max_ps(a, b));
      }
      std::array<float, 4> out;
      _mm_storeu_ps(&out[0], simd);
      //
      float result = out[0];
      for (size_t i = 1; i < out.size(); ++i)
         if (out[i] > result)
            result = out[i];
      return result;
   }

   template<template<typename, size_t> typename Container, size_t Size> requires impl::min_and_max::is_bounded_float_container<Container, Size>
   constexpr float minimum_of_list(const Container<float, Size>& list) {
      constexpr auto initial = std::numeric_limits<float>::max();
      //
      size_t i = 0;
      //
      auto simd = _mm_set_ps1(initial);
      for (; i + 7 < Size; i += 8) {
         auto a = _mm_loadu_ps(&list[i + 0]);
         auto b = _mm_loadu_ps(&list[i + 4]);
         simd = _mm_min_ps(simd, _mm_min_ps(a, b));
      }
      std::array<float, 4> out;
      _mm_storeu_ps(&out[0], simd);
      //
      float result = out[0];
      for (size_t i = 1; i < out.size(); ++i)
         if (out[i] < result)
            result = out[i];
      return result;
   }
}