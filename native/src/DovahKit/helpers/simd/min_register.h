#pragma once
#include <intrin.h>

namespace cobb::simd {
   inline __m128 min_register(__m128 v) {
      v = _mm_min_ps(v, _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 1, 0, 3)));
      v = _mm_min_ps(v, _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 0, 3, 2)));
      return v;
   }
}