#pragma once
#include "./position.h"

namespace cobb::bitstreams {
   constexpr position& position::advance_by_bits(size_t bits) {
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
      // for a full_position, caller will likely want to clamp_to_size at this point, too
      return *this;
   }
   constexpr position& position::advance_by_bytes(size_t bytes) {
      this->bits   = 0;
      this->bytes += bytes;
      return *this;
   }
   constexpr position& position::advance_to_next_byte() {
      this->bits = 0;
      ++this->bytes;
      // for a full_position, caller will likely want to clamp_to_size at this point, too
      return *this;
   }

   constexpr position& position::rewind_by_bits(size_t bits) {
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
   constexpr position& position::rewind_by_bytes(size_t bytes) {
      this->bytes -= bytes;
      return *this;
   }
}