#pragma once
#include <intrin.h>

namespace cobb::simd {
   template<unsigned int a, unsigned int b, unsigned int c, unsigned int d> requires (a < 4 && b < 4 && c < 4 && d < 4)
   inline __m128 shuffle(__m128 v) {
      return _mm_shuffle_ps(v, v, _MM_SHUFFLE(d, c, b, a));
   }
}