#pragma once
#include <intrin.h>

namespace cobb::simd {
   template<bool sse3 = true> __m128 sum_register_to_register(__m128 v) {
      if constexpr (sse3) {
         auto halves = _mm_hadd_ps(v, v); // [ x+y, z+w, x+y, z+w ]
         return _mm_hadd_ps(halves, halves);
      } else {
         auto temp = _mm_add_ps(_mm_movehl_ps(v, v), v); // add the low and high pairs of floats
         return _mm_add_ss(temp, _mm_shuffle_ps(temp, temp, _MM_SHUFFLE(0, 0, 0, 1)));
      }
   }
   template<bool sse3 = true> float sum_register(__m128 v) {
      return _mm_cvtss_f32(sum_float4_register_to_register<sse3>(v));
   }
}