#include "./matrix.h"
#include <cstdint>
#include <immintrin.h>

namespace cobb::impl::matrix_intrinsics {
   extern bool is_diagonal_2x2(const float* data) {
      constexpr const uint32_t nonzero_signed = 0b0'11111111'11111111111111111111111;
      // A floating-point value is (+/-)zero if the exponent and significand are both zero.

      auto cmp_0110 = _mm_castsi128_ps(_mm_set_epi32(0, nonzero_signed, nonzero_signed, 0));

      auto src = _mm_load_ps(data);
      src = _mm_and_ps(src, cmp_0110);

      auto cmp = _mm_cmpeq_epi8(_mm_castps_si128(src), _mm_setzero_si128());
      #if __SSE4_1__
         if (_mm_testz_si128(cmp, cmp))
            return false;
      #else
         if (_mm_movemask_epi8(cmp) != 0)
            return false;
      #endif
      return true;
   }

   extern bool is_diagonal_3x3(const float* data) {
      constexpr const uint32_t nonzero_signed = 0b0'11111111'11111111111111111111111;
      // A floating-point value is (+/-)zero if the exponent and significand are both zero.

      // a b c
      // d e f
      // g h i

      auto cmp_0111 = _mm_castsi128_ps(_mm_set_epi32(0, nonzero_signed, nonzero_signed, nonzero_signed));

      auto src_abcd = _mm_load_ps(data);
      auto src_efgh = _mm_load_ps(data + 4);
      src_abcd = _mm_and_ps(src_abcd, cmp_0111);
      src_efgh = _mm_and_ps(src_efgh, cmp_0111);

      auto cmp_a = _mm_cmpeq_epi8(_mm_castps_si128(src_abcd), _mm_setzero_si128());
      auto cmp_b = _mm_cmpeq_epi8(_mm_castps_si128(src_efgh), _mm_setzero_si128());
      cmp_a = _mm_and_si128(cmp_a, cmp_b);
      #if __SSE4_1__
      if (_mm_testz_si128(cmp_a, cmp_a))
         return false;
      #else
      if (_mm_movemask_epi8(cmp_a) != 0)
         return false;
      #endif
      return true;
   }

   extern void transpose_4x4(float* data) {
      auto row_a = _mm_load_ps(data);
      auto row_b = _mm_load_ps(data + 4);
      auto row_c = _mm_load_ps(data + 8);
      auto row_d = _mm_load_ps(data + 12);
      _MM_TRANSPOSE4_PS(row_a, row_b, row_c, row_d);
      _mm_store_ps(data,      row_a);
      _mm_store_ps(data +  4, row_b);
      _mm_store_ps(data +  8, row_c);
      _mm_store_ps(data + 12, row_d);
   }
   extern void transpose_4x4(float* src, float* dst) {
      auto row_a = _mm_load_ps(src);
      auto row_b = _mm_load_ps(src +  4);
      auto row_c = _mm_load_ps(src +  8);
      auto row_d = _mm_load_ps(src + 12);
      _MM_TRANSPOSE4_PS(row_a, row_b, row_c, row_d);
      _mm_store_ps(dst,      row_a);
      _mm_store_ps(dst +  4, row_b);
      _mm_store_ps(dst +  8, row_c);
      _mm_store_ps(dst + 12, row_d);
   }
}