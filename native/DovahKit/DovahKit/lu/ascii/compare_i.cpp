#include "./compare_i.h"
#include <bit>
#include <intrin.h>
#include "../cpuinfo.h"

#include "./_macros/CASE_FOLD_REGISTERS_IN_TANDEM.h"

#define IF_DIFFERENT_THEN_RETURN_SIGN(ma, mb) \
   { \
      uint16_t eq = _mm_movemask_epi8(_mm_cmpeq_epi8(ma, mb)); \
      if (eq != 0xFFFF) { \
         auto j = std::countr_one(eq); \
         return (int)(unsigned char)a[i + j] - (int)(unsigned char)b[i + j]; \
      } \
   }

namespace lu::ascii {
   extern int compare_i_simd(const std::string_view a, const std::string_view b) {
      const size_t size_a   = a.size();
      const size_t size_b   = b.size();
      const size_t size_min = (size_a < size_b) ? size_a : size_b;

      // help perf for debug builds where inlining never happens:
      const auto* a_data = a.data();
      const auto* b_data = b.data();

      size_t i = 0;
      const auto& cpu = cpuinfo::get();
      if (cpu.extension_support.sse_2 && cpu.extension_support.sse_3) {
         auto mb_a = _mm_set1_epi8('A' - 1);
         auto mb_z = _mm_set1_epi8('Z' + 1);
         for (; i + 15 < size_min; i += 16) {
            auto ma = _mm_loadu_si128((const __m128i*)(a_data + i));
            auto mb = _mm_loadu_si128((const __m128i*)(b_data + i));
            CASE_FOLD_REGISTERS_IN_TANDEM(ma, mb);
            IF_DIFFERENT_THEN_RETURN_SIGN(ma, mb);
         }
         if (i + 7 < size_min) {
            auto ma = _mm_loadl_epi64((const __m128i*)(a_data + i));
            auto mb = _mm_loadl_epi64((const __m128i*)(b_data + i));
            CASE_FOLD_REGISTERS_IN_TANDEM(ma, mb);
            IF_DIFFERENT_THEN_RETURN_SIGN(ma, mb);
            
            i += 8;
         }
      }
      for (; i < size_min; ++i) {
         unsigned char x = a_data[i];
         unsigned char y = b_data[i];
         if (x >= 'A' && x <= 'Z')
            x |= 0x20;
         if (y >= 'A' && y <= 'Z')
            y |= 0x20;
         auto diff = (int)x - y;
         if (diff)
            return diff;
      }
      if (size_a != size_b) {
         if (size_a == size_min)
            return -(int)(unsigned char)b[size_a];
         return (unsigned char)a[size_b];
      }
      return 0;
   }
}