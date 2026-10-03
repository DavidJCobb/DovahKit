#pragma once
#include <bit>
#include "lu/concepts/instantiation_of/std_array.h"
#include "lu/enums/enum_flags.h"
#include "lu/integral_character_code.h"

namespace dovah::tes::impl {
   template<typename T>
   void enforce_endianness(bool needs_endianness_swap, T& dst) {
      if constexpr (sizeof(T) <= 1) {
         return;
      }

      if constexpr (lu::integral_character_code_type<T>) {
         //
         // Four-CCs have opposite endianness to everything else.
         //
         if (!needs_endianness_swap) {
            dst.value = std::byteswap(dst.value);
            return;
         }
      }

      if (!needs_endianness_swap)
         return;
      if constexpr (lu::concepts::instantiation_of_std_array<T>) {
         for (auto& item : dst)
            byteswap(item);
      } else if constexpr (lu::enum_flags_type<T>) {
         auto mask = (typename T::mask_type)dst;
         dst = T::from_mask(std::byteswap(mask));
      } else {
         dst = std::byteswap(dst);
      }
   }
}