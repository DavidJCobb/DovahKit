#include "mat4_by_vec4_simd.h"
#include <intrin.h>

namespace cobb::glm {
   extern ::glm::vec4 mat4_by_vec4_simd(const ::glm::mat4& m, const ::glm::vec4& v) {
      __m128 simd_v  = _mm_load_ps(&v.x);
      __m128 simd_vx = _mm_shuffle_ps(simd_v, simd_v, _MM_SHUFFLE(0, 0, 0, 0));
      __m128 simd_vy = _mm_shuffle_ps(simd_v, simd_v, _MM_SHUFFLE(1, 1, 1, 1));
      __m128 simd_vz = _mm_shuffle_ps(simd_v, simd_v, _MM_SHUFFLE(2, 2, 2, 2));
      __m128 simd_vw = _mm_shuffle_ps(simd_v, simd_v, _MM_SHUFFLE(3, 3, 3, 3));
      //
      __m128 a = _mm_mul_ps(_mm_load_ps(&m[0].x), simd_vx);
      __m128 b = _mm_mul_ps(_mm_load_ps(&m[1].x), simd_vy);
      __m128 c = _mm_add_ps(a, b);
      a = _mm_mul_ps(_mm_load_ps(&m[2].x), simd_vz);
      b = _mm_mul_ps(_mm_load_ps(&m[3].x), simd_vw);
      __m128 d = _mm_add_ps(a, b);
      //
      ::glm::vec4 out;
      _mm_store_ps(&out.x, _mm_add_ps(c, d));
      return out;
   }
}