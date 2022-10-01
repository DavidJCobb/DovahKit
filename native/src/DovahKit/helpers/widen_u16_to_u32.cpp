#include "widen_u16_to_u32.h"
#include <intrin.h>
#include "./cpuinfo.h"

namespace cobb {
   extern void widen_u16_to_u32(size_t count, const uint16_t* src_array, uint32_t* dst_array) {
      size_t i = 0;
      if (auto& c = cobb::cpuinfo::get().extension_support; c.sse_2 && c.sse_4_1) {
         for (; i + 7 < count; i += 8) {
            auto here = (std::intptr_t)&dst_array[i];
            auto half = (std::intptr_t)&dst_array[i] + (sizeof(uint32_t) * 4);

            __m128i src = _mm_loadu_si128((__m128i*)&src_array[i]);
            __m128i dst = _mm_cvtepu16_epi32(src); // convert first four uint16_ts
            _mm_storeu_si128((__m128i*)here, dst);
            src = _mm_srli_si128(src, 8);  // shift by 8 bytes
            dst = _mm_cvtepu16_epi32(src); // convert second four uint16_ts
            _mm_storeu_si128((__m128i*)half, dst);
         }
      }
      for (; i < count; ++i) {
         dst_array[i] = static_cast<uint32_t>(src_array[i]);
      }
   }
}