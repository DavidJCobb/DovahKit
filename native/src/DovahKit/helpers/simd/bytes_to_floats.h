#pragma once
#include <cstdint>
#include <intrin.h>
#include "../endian.h"

namespace cobb::simd {
   __m128 bytes_to_floats(uint32_t v) {
      v = cobb::endian_cast<std::endian::little>(v);
      auto i = _mm_loadu_si32(&v);
      return _mm_cvtepi32_ps(_mm_cvtepu8_epi32(i));
   }
}