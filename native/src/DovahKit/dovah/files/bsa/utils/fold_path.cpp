#include "./fold_path.h"
#include <immintrin.h>
#include "helpers/cpuinfo.h"
#include "../constants/path_separators.h"

namespace dovah::bsa::utils {
   extern void fold_path(std::string& out) {
      if (out.empty())
         return;
      auto*  data = out.data();
      size_t size = out.size();
      size_t i    = 0;
      //
      if (auto& cpu = cobb::cpuinfo::get(); cpu.extension_support.sse_2 && cpu.extension_support.sse_3) {
         auto mb_a = _mm_set1_epi8('A' - 1);
         auto mb_z = _mm_set1_epi8('Z' + 1);
         auto mb_s = _mm_set1_epi8(secondary_path_separator);
         auto fill = _mm_set1_epi8(preferred_path_separator);
         for (; i + 15 < size; i += 16) {
            auto ma = _mm_loadu_si128((const __m128i*)(data + i));
            //
            //
            // Goal: for each active byte in (mask_a), OR the byte in (ma) by 0x20
            //
            auto mask_a = _mm_cmpgt_epi8(ma, mb_a); // per byte: (a >= 'A') ? 0xFF : 0
            auto mask_z = _mm_cmplt_epi8(ma, mb_z); // per byte: (a <= 'Z') ? 0xFF : 0
            mask_a = _mm_and_si128(mask_a, mask_z); // bitwise-AND
            mask_a = _mm_and_si128(_mm_set1_epi8(0x20), mask_a); // per byte: (a >= 'A' && a <= 'Z') ? 0x20 : 0
            ma = _mm_or_si128(ma, mask_a); // bitwise-OR
            //
            // Goal: for each forward slash in (ma), convert it to a backslash.
            //
            mask_a = _mm_cmpeq_epi8(ma, mb_s);
            ma = _mm_blendv_epi8(ma, fill, mask_a); // per byte: dst = (mask & 0x80) ? b : a
            //
            _mm_storeu_si128((__m128i*)(data + i), ma);
         }
         if (i + 7 < size) {
            auto ma = _mm_loadl_epi64((const __m128i*)(data + i));
            //
            auto mask_a = _mm_cmpgt_epi8(ma, mb_a); // per byte: (a >= 'A') ? 0xFF : 0
            auto mask_z = _mm_cmplt_epi8(ma, mb_z); // per byte: (a <= 'Z') ? 0xFF : 0
            mask_a = _mm_and_si128(mask_a, mask_z); // bitwise-AND
            mask_a = _mm_and_si128(_mm_set1_epi8(0x20), mask_a); // per byte: (a >= 'A' && a <= 'Z') ? 0x20 : 0
            ma = _mm_or_si128(ma, mask_a); // bitwise-OR
            //
            mask_a = _mm_cmpeq_epi8(ma, mb_s);
            ma = _mm_blendv_epi8(ma, fill, mask_a); // per byte: dst = (mask & 0x80) ? b : a
            //
            _mm_storel_epi64((__m128i*)(data + i), ma);
            //
            i += 8;
         }
      }
      for (; i < size; ++i) {
         auto c = data[i];
         if (c >= 'A' && c <= 'Z')
            data[i] = c | 0x20;
         else if (c == secondary_path_separator)
            c = preferred_path_separator;
      }
   }
}