#pragma once
#include "./bitstream_position.h"

namespace cobb::streams {
   #pragma region bitstream_position
   constexpr bitstream_position& bitstream_position::advance_by_bits(size_t bits) {
      if (this->bits) {
         size_t to_consume = 8 - this->bits;
         if (bits < to_consume) {
            this->bits += bits;
            return *this;
         }
         this->bits   = 0;
         this->bytes += 1;
         bits -= to_consume;
      }
      if (bits) {
         this->bytes += bits / 8;
         this->bits   = bits % 8;
      }
      // for a full_bitstream_position, caller will likely want to clamp_to_size at this point, too
      return *this;
   }
   constexpr bitstream_position& bitstream_position::advance_by_bytes(size_t bytes) {
      this->bits   = 0;
      this->bytes += bytes;
      return *this;
   }
   constexpr bitstream_position& bitstream_position::advance_to_next_byte() {
      this->bits = 0;
      ++this->bytes;
      // for a full_bitstream_position, caller will likely want to clamp_to_size at this point, too
      return *this;
   }

   constexpr bitstream_position& bitstream_position::rewind_by_bits(size_t bits) {
      if (this->bits > 0) {
         if (this->bits > bits) {
            this->bits -= bits;
            return *this;
         }
         bits -= this->bits;
         this->bits = 0;
      }
      this->bytes -= (bits / 8);
      bits = bits % 8;
      if (bits) {
         --this->bytes;
         this->bits = 8 - bits;
      }
      return *this;
   }
   constexpr bitstream_position& bitstream_position::rewind_by_bytes(size_t bytes) {
      this->bytes -= bytes;
      return *this;
   }
   #pragma endregion

   #pragma region full_bitstream_position
   constexpr void full_bitstream_position::add_overshoot_bits(size_t b) {
      this->overshoot.advance_by_bits(b);
   }
   constexpr void full_bitstream_position::clamp_to_size(size_t size) {
      if (this->bytes <= size)
         return;
      this->overshoot = {
         .bytes = (this->bytes - size),
         .bits = this->bits,
      };
      this->bytes = size;
      this->bits = 0;
   }
   #pragma endregion
}