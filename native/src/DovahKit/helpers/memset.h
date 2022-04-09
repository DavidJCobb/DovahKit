#pragma once
#include <bit>
#include <cstdint>
#include <type_traits>
#include "cpuinfo.h"
#include <intrin.h>

namespace cobb {
   template<typename Dword> requires requires {
      requires sizeof(Dword) == 4;
      requires !std::is_pointer_v<Dword>;
   } void memset(void* dst, const Dword v, size_t count) {
      auto* typed_dst = (uint32_t*)dst;
      //
      size_t i = 0;
      if (cobb::cpuinfo::get().extension_support.sse_2) {
         __m128i simd = _mm_set1_epi32(std::bit_cast<uint32_t>(v));
         for (; i + 3 < count; i += 4) {
            _mm_storeu_si128((__m128i*)typed_dst, simd);
            typed_dst += 4;
         }
      }
      for (; i < count; ++i) {
         *typed_dst = v;
         ++typed_dst;
      }
   }
}