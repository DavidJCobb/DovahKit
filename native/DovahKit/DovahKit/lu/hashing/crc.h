#pragma once
#include <cstddef> // size_t
#include <cstdint>
#include "../bit/reverse.h"
#include "./crc/definition.h"
#include "./crc/definition_value_type.h"
#include "./crc/lookup_table.h"

namespace lu::hashing::crc {
   constexpr uint8_t no_op_byte_transform(uint8_t v) noexcept {
      return v;
   }

   template<rocksoft_definition Definition, auto PreTransformByteCallable = &no_op_byte_transform>
      requires (Definition.bitcount > 0)
   constexpr definition_value_type<Definition> hash(const uint8_t* data, const size_t size) {
      using value_type = definition_value_type<Definition>;

      value_type crc = Definition.initial;
      if constexpr (Definition.bitcount > 8) {
         constexpr const auto& table = lookup_table<Definition>;
         for (size_t i = 0; i < size; ++i) {
            auto byte = data[i];
            if constexpr (PreTransformByteCallable != &no_op_byte_transform) {
               byte = PreTransformByteCallable(byte);
            }
            if constexpr (Definition.reflect_input_bytes) {
               crc = table[(crc ^ byte) & 0xFF] ^ (crc >> 8);
            } else {
               crc = table[(crc >> 24) ^ byte] ^ (crc << 8);
            }
         }
         if constexpr (Definition.reflect_input_bytes != Definition.reflect_output_register) {
            crc = bits::reverse(crc);
         }
      } else {
         //
         // Register bitcount is too small to even use a lookup table, so we have to 
         // compute the hash bit by bit.
         // 
         // Basic idea per: <https://www.kernel.org/doc/html/v6.6/staging/crc32.html>
         // Follows the example set by the little-endian code described therein.
         //
         constexpr const value_type top_bit = (value_type)1 << (Definition.bitcount - 1);
         constexpr const value_type mask    = top_bit | (top_bit - 1);
         for (size_t i = 0; i < size; ++i) {
            auto byte = data[i];
            if constexpr (PreTransformByteCallable != &no_op_byte_transform) {
               byte = PreTransformByteCallable(byte);
            }
            for (size_t j = 0; j < 8; ++j) {
               uint8_t bit = 0;
               if constexpr (Definition.reflect_input_bytes) {
                  bit = (byte >> (8 - j - 1)) & 1;
               } else {
                  bit = (byte >> j) & 1;
               }

               crc ^= bit;
               value_type multiple = (crc & 1) ? Definition.poly : 0;
               crc ^= (crc >> 1) ^ multiple;

               crc &= mask;
            }
         }
         if constexpr (Definition.reflect_input_bytes != Definition.reflect_output_register) {
            crc = bits::reverse_with_bitcount<Definition.bitcount>(crc);
         }
      }

      return crc ^ Definition.xor_output;
   }
}
