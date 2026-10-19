#pragma once
#include <bit>
#include <cstdint>
#include <concepts>
#include "../concepts/instantiation_of/std_array.h"

namespace lu::bits {
   namespace impl {
      extern void byteswap_in_place_simd(uint16_t* dst, size_t count);
      extern void byteswap_in_place_simd(uint32_t* dst, size_t count);
      extern void byteswap_in_place_simd(uint64_t* dst, size_t count);
   }

   template<std::integral T>
   constexpr void byteswap_in_place(T& dst) {
      dst = std::byteswap(dst);
   }

   template<concepts::instantiation_of_std_array T>
      requires requires {
         typename T::value_type;
         requires std::integral<typename T::value_type>;
      }
   constexpr void byteswap_in_place(T& dst) {
      using value_type = T::value_type;
      constexpr size_t value_size = sizeof(value_type);

      if constexpr (value_size > 1) {
         if (!std::is_constant_evaluated()) {
            if constexpr (value_size == 2) {
               impl::byteswap_in_place_simd((uint16_t*)dst.data(), dst.size());
               return;
            } else if constexpr (value_size == 4) {
               impl::byteswap_in_place_simd((uint32_t*)dst.data(), dst.size());
               return;
            } else if constexpr (value_size == 8) {
               impl::byteswap_in_place_simd((uint64_t*)dst.data(), dst.size());
               return;
            }
         }
         for (size_t i = 0; i < dst.size(); ++i)
            byteswap_in_place(dst[i]);
      }
   }
}
