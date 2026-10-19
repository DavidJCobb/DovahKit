#include "./byteswap_in_place.h"
#include "../cpuinfo.h"
#include <memory>
#include <intrin.h>

template<typename T>
static void _byteswap_in_place_simd(T* dst, size_t count) {
   constexpr const size_t bytes_per_register = 16;
   constexpr const size_t count_per_register = bytes_per_register / sizeof(T);

   if (count < count_per_register) {
      for (size_t i = 0; i < count; ++i)
         dst[i] = std::byteswap(dst[i]);
      return;
   }

   auto misalign = (std::uintptr_t)dst % bytes_per_register;
   if (misalign % sizeof(T)) {
      //
      // Pointer is wholly misaligned; we cannot byteswap "naturally."
      //
      for (size_t i = 0; i < count; ++i)
         dst[i] = std::byteswap(dst[i]);
      return;
   }
   misalign = (bytes_per_register - misalign) / sizeof(T);

   for (size_t i = 0; i < misalign; ++i) {
      dst[i] = std::byteswap(dst[i]);
   }
   dst    = std::assume_aligned<16>(dst + misalign);
   count -= misalign;

   if (count >= count_per_register) {
      __m128i shuffle_mask;
      if constexpr (sizeof(T) == 2) {
         shuffle_mask = _mm_set_epi8(
            14, 15,
            12, 13,
            10, 11,
             8,  9,
             6,  7,
             4,  5,
             2,  3,
             0,  1
         );
      } else if constexpr (sizeof(T) == 4) {
         shuffle_mask = _mm_set_epi8(
            12, 13, 14, 15,
             8,  9, 10, 11,
             4,  5,  6,  7,
             0,  1,  2,  3
         );
      } else if constexpr (sizeof(T) == 8) {
         shuffle_mask = _mm_set_epi8(
            8, 9, 10, 11, 12, 13, 14, 15,
            0, 1,  2,  3,  4,  5,  6,  7
         );
      }
      while (count >= count_per_register) {
         _mm_storeu_si128(
            (__m128i*)dst,
            _mm_shuffle_epi8(_mm_loadu_si128((__m128i*)dst), shuffle_mask)
         );
         dst   += count_per_register;
         count -= count_per_register;
      }
   }

   for (size_t i = 0; i < count; ++i) {
      dst[i] = std::byteswap(dst[i]);
   }
}

namespace lu::bits::impl {
   extern void byteswap_in_place_simd(uint16_t* dst, size_t count) {
      _byteswap_in_place_simd(dst, count);
   }
   extern void byteswap_in_place_simd(uint32_t* dst, size_t count) {
      _byteswap_in_place_simd(dst, count);
   }
   extern void byteswap_in_place_simd(uint64_t* dst, size_t count) {
      _byteswap_in_place_simd(dst, count);
   }
}