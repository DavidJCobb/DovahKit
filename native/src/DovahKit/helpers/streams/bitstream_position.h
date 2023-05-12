#pragma once
#include <cstdint>

namespace cobb::streams {
   struct bitstream_position {
      size_t  bytes = 0;
      uint8_t bits = 0;

      constexpr size_t bytespan() const noexcept { return this->bytes + (this->bits ? 1 : 0); }
      constexpr size_t in_bits()  const noexcept { return (this->bytes * 8) + this->bits; }

      constexpr bitstream_position& advance_by_bits(size_t bits);
      constexpr bitstream_position& advance_by_bytes(size_t bits);
      constexpr bitstream_position& advance_to_next_byte();
      //
      constexpr bitstream_position& rewind_by_bits(size_t bits);
      constexpr bitstream_position& rewind_by_bytes(size_t bytes);

      constexpr bitstream_position advanced_by_bits(size_t b) const { return bitstream_position(*this).advance_by_bits(b); }
      constexpr bitstream_position advanced_by_bytes(size_t b) const { return bitstream_position(*this).advance_by_bytes(b); }
      constexpr bitstream_position advanced_to_next_byte() const { return bitstream_position(*this).advance_to_next_byte(); }
      //
      constexpr bitstream_position rewound_by_bits(size_t b) const { return bitstream_position(*this).rewind_by_bits(b); }
      constexpr bitstream_position rewound_by_bytes(size_t b) const { return bitstream_position(*this).rewind_by_bytes(b); }

      constexpr void set_in_bits(size_t bits) {
         this->bytes = bits / 8;
         this->bits  = bits % 8;
      }
      constexpr void set_in_bytes(size_t bytes) {
         this->bytes = bytes;
         this->bits  = 0;
      }
   };

   struct full_bitstream_position : public bitstream_position {
      bitstream_position overshoot;

      constexpr size_t in_bits() const noexcept { return bitstream_position::in_bits() + this->overshoot.in_bits(); }

      constexpr size_t overshoot_in_bits() const noexcept { return this->overshoot.in_bits(); }
      constexpr size_t overshoot_in_bytes() const noexcept { return this->overshoot.bytes; }

      constexpr void clamp_to_size(size_t);

      constexpr void set_in_bits(size_t b) {
         bitstream_position::set_in_bits(b);
         this->overshoot = {};
      }
      constexpr void set_in_byte(size_t b) {
         bitstream_position::set_in_bytes(b);
         this->overshoot = {};
      }

      constexpr void add_overshoot_bits(size_t);
      constexpr void set_overshoot_bits(size_t b) { this->overshoot.set_in_bits(b); }
      constexpr void set_overshoot_bytes(size_t b) { this->overshoot.set_in_bytes(b); }
   };
}

#include "./bitstream_position.inl"