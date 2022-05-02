#include "cross_simd.h"
#include <intrin.h>

namespace {
   // Same as the _MM_SHUFFLE macro, but values aren't in reverse order, and there's some compile-time checking.
   template<unsigned int a, unsigned int b, unsigned int c, unsigned int d> requires (a < 4 && b < 4 && c < 4 && d < 4)
   constexpr int _shuffle_mask = ([]() {
      return _MM_SHUFFLE(d, c, b, a);
   })();
}

namespace cobb::glm {
   extern ::glm::vec4 cross_simd(const ::glm::vec4& a, const ::glm::vec4& b) {
      ::glm::vec4 out;
      _mm_storeu_ps(&out.x, cross_simd_to_register(a, b));
      return out;
   }
   extern __m128 cross_simd_to_register(const ::glm::vec4& a, const ::glm::vec4& b) {
      auto a_simd = _mm_loadu_ps(&a.x);
      auto b_simd = _mm_loadu_ps(&b.x);
      //
      // The final goal:
      // 
      //    out.x = a.y * b.z - a.z * b.y
      //    out.y = a.z * b.x - a.x * b.z
      //    out.z = a.x * b.y - a.y * b.x
      //
      // Let's depict that another way:
      // 
      //      [ a.y * b.z , a.z * b.x , a.x * b.y ]
      //    - [ a.z * b.y , a.x * b.z , a.y * b.x ]
      //
      // We'll need to shuffle the values in order to do a SIMD multiplication. The 
      // naive approach would be to shuffle everything, but what if we just shuffle 
      // the "b" vector so that we can multiply the "a" vector directly in?
      // 
      //    c == [ a.x * b.y , a.y * b.z , a.z * b.x ]
      //    d == [ a.x * b.z , a.y * b.x , a.z * b.y ]
      //
      auto c = _mm_shuffle_ps(b_simd, b_simd, _shuffle_mask<1, 2, 0, 3>); // b-components: y, z, x, w
      auto d = _mm_shuffle_ps(b_simd, b_simd, _shuffle_mask<2, 0, 1, 3>); // b-components: z, x, y, w
      c = _mm_mul_ps(a_simd, c);
      d = _mm_mul_ps(a_simd, d);
      //
      // What we want next is:
      // 
      //    out.x = c.y - d.z
      //    out.y = c.z - d.x
      //    out.z = c.x - d.y
      //
      auto o_simd = _mm_sub_ps(
         _mm_shuffle_ps(c, c, _shuffle_mask<1, 2, 0, 3>),
         _mm_shuffle_ps(d, d, _shuffle_mask<2, 0, 1, 3>)
      );
      //
      // Finally, let's clear the W-component (since there's no such things as a 
      // 4D cross product; we only use vec4s for SIMD) and return the whole vector.
      //
      o_simd = _mm_or_ps(o_simd, _mm_set_ps(1, 1, 1, 0));
      return o_simd;
   }
}