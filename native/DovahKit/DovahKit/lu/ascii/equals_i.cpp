#include "./equals_i.h"
#include <intrin.h>
#include "../cpuinfo.h"

#include "./_macros/ARE_128_BIT_REGISTERS_DIFFERENT.h"
#include "./_macros/CASE_FOLD_REGISTERS_IN_TANDEM.h"

namespace lu::ascii {
   extern bool equals_i_simd(const std::string_view a, const std::string_view b) {
      const size_t size = a.size();
      if (size != b.size())
         return false;

      // help perf for debug builds where inlining never happens:
      const auto* a_data = a.data();
      const auto* b_data = b.data();
      
      size_t i = 0;
      const auto& cpu = cpuinfo::get();
      if (cpu.extension_support.sse_2 && cpu.extension_support.sse_3) {
         auto mb_a = _mm_set1_epi8('A' - 1);
         auto mb_z = _mm_set1_epi8('Z' + 1);
         for (; i + 15 < size; i += 16) {
            auto ma = _mm_loadu_si128((const __m128i*)(a_data + i));
            auto mb = _mm_loadu_si128((const __m128i*)(b_data + i));
            CASE_FOLD_REGISTERS_IN_TANDEM(ma, mb);
            if (ARE_128_BIT_REGISTERS_DIFFERENT(ma, mb)) {
               return false;
            }
         }
         if (i + 7 < size) {
            auto ma = _mm_loadl_epi64((const __m128i*)(a_data + i));
            auto mb = _mm_loadl_epi64((const __m128i*)(b_data + i));
            CASE_FOLD_REGISTERS_IN_TANDEM(ma, mb);
            if (ARE_128_BIT_REGISTERS_DIFFERENT(ma, mb)) {
               return false;
            }
            
            i += 8;
         }
      }
      for (; i < size; ++i) {
         char x = a_data[i];
         char y = b_data[i];
         if (x >= 'A' && x <= 'Z')
            x |= 0x20;
         if (y >= 'A' && y <= 'Z')
            y |= 0x20;
         if (x != y)
            return false;
      }
      return true;
   }
}