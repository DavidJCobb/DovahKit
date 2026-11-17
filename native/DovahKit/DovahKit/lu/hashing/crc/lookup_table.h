#pragma once
#include <array>
#include "../../bit/reverse.h"
#include "./definition.h"
#include "./definition_value_type.h"

namespace lu::hashing::crc {
   // The produced table's entries will be reflected if the input is reflected.
   template<rocksoft_definition Definition>
      requires (Definition.bitcount > 8)
   inline constexpr const std::array<definition_value_type<Definition>, 256> lookup_table = []() {
      using value_type = definition_value_type<Definition>;
      constexpr const value_type top_bit = ((value_type)1 << (Definition.bitcount - 1));

      std::array<value_type, 256> table = {};

      for (size_t i = 0; i < table.size(); ++i) {
         value_type input_byte = i;
         if constexpr (Definition.reflect_input_bytes) {
            input_byte = bits::reverse((uint8_t)i);
         }
         value_type reg = input_byte << (Definition.bitcount - 8);
         for (size_t j = 0; j < 8; ++j) {
            if (reg & top_bit) {
               reg = (reg << 1) ^ Definition.poly;
            } else {
               reg <<= 1;
            }
            if constexpr (sizeof(reg) * 8 > Definition.bitcount) {
               reg &= (((value_type)1 << Definition.bitcount) - 1);
            }
         }
         if constexpr (Definition.reflect_input_bytes) {
            reg = bits::reverse(reg);
         }
         table[i] = reg;
      }

      return table;
   }();
}
