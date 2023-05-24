#pragma once
#include <cstdint>

namespace cobb::bitstreams {
   struct position {
      size_t  bytes = 0;
      uint8_t bits  = 0;

      constexpr size_t bytespan() const noexcept { return this->bytes + (this->bits ? 1 : 0); }
      constexpr size_t in_bits()  const noexcept { return (this->bytes * 8) + this->bits; }

      constexpr position& advance_by_bits(size_t bits);
      constexpr position& advance_by_bytes(size_t bits);
      constexpr position& advance_to_next_byte();
      //
      constexpr position& rewind_by_bits(size_t bits);
      constexpr position& rewind_by_bytes(size_t bytes);

      [[nodiscard("This function creates a new instance; use the present-tense name to modify the current instance.")]]
      constexpr position advanced_by_bits(size_t b) const { return position(*this).advance_by_bits(b); }
      //
      [[nodiscard("This function creates a new instance; use the present-tense name to modify the current instance.")]]
      constexpr position advanced_by_bytes(size_t b) const { return position(*this).advance_by_bytes(b); }
      //
      [[nodiscard("This function creates a new instance; use the present-tense name to modify the current instance.")]]
      constexpr position advanced_to_next_byte() const { return position(*this).advance_to_next_byte(); }
      //
      // Rewinds:
      //
      [[nodiscard("This function creates a new instance; use the present-tense name to modify the current instance.")]]
      constexpr position rewound_by_bits(size_t b) const { return position(*this).rewind_by_bits(b); }
      //
      [[nodiscard("This function creates a new instance; use the present-tense name to modify the current instance.")]]
      constexpr position rewound_by_bytes(size_t b) const { return position(*this).rewind_by_bytes(b); }

      constexpr void set_in_bits(size_t bits) {
         this->bytes = bits / 8;
         this->bits  = bits % 8;
      }
      constexpr void set_in_bytes(size_t bytes) {
         this->bytes = bytes;
         this->bits  = 0;
      }
   };
}

#include "./position.inl"