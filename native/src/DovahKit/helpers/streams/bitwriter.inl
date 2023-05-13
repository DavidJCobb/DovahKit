#pragma once
#include "./bitwriter.h"

#include <stdexcept>
#include <type_traits>
#include "../bitwise.h"

namespace cobb::streams {
   constexpr bitwriter::~bitwriter() {
      this->_buffer_free();
      this->_size     = 0;
      this->_position = {};
   }
   
   constexpr void bitwriter::_buffer_free() {
      if (!this->_buffer)
         return;
      if (std::is_constant_evaluated()) {
         delete[] this->_buffer;
      } else {
         free(this->_buffer);
      }
      this->_buffer = nullptr;
   }

   constexpr uint8_t& bitwriter::_access_byte(size_t bytepos) const noexcept {
      return this->_buffer[bytepos];
   }
   constexpr void bitwriter::_ensure_room_for(unsigned int bitcount) {
      size_t bitsize = (size_t)this->_size * 8;
      size_t target  = (size_t)this->get_bitpos() + bitcount;
      if (target > bitsize) {
         target += 8 - (target % 8);
         this->resize(target / 8);
      }
   }

   constexpr void bitwriter::reserve(size_t size) {
      if (size < this->_size)
         return;
      this->resize(size);
   }
   constexpr void bitwriter::resize(size_t size) {
      if (this->_size == size)
         return;
      if (size == 0) {
         this->_buffer_free();
      } else {
         if (std::is_constant_evaluated()) {
            auto* after = new std::uint8_t[size]{};
            if (this->_buffer) {
               size_t end = this->_size;
               if (size < end)
                  end = size;
               for (size_t i = 0; i < end; ++i)
                  after[i] = this->_buffer[i];
               delete[] this->_buffer;
            }
            this->_buffer = after;
         } else {
            auto* prior = this->_buffer;
            this->_buffer = (uint8_t*)realloc(this->_buffer, size);
            if (this->_buffer == nullptr) {
               this->_buffer = prior;
               throw std::bad_alloc();
            }
         }
      }
      this->_size = size;
      if (size <= this->_position.bytes) {
         this->_position.bytes = size;
         this->_position.bits  = 0;
      }
   }

   constexpr void bitwriter::write(const std::string& v) {
      using char_type = std::decay_t<decltype(v)>::value_type;

      const auto size = v.size();
      this->write_bits<length_prefix_serialized_type>(sizeof(length_prefix_serialized_type) * 8, size);

      if (!std::is_constant_evaluated()) {
         if (this->is_byte_aligned() && size + this->get_bytepos() <= this->size()) {
            memcpy(this->_buffer + this->get_bytepos(), v.data(), size * sizeof(char_type));
            this->_position.advance_by_bytes(size * sizeof(char_type));
            //
            return;
         }
      }
      /*// IntelliSense chokes and dies on range-based for loops used on std::string and friends during compile-time evaluation.
      for (auto& c : v)
         this->write(c);
      //*/
      for (size_t i = 0; i < v.size(); ++i)
         this->write(v[i]);
   }
   constexpr void bitwriter::write(const std::wstring& v) {
      using char_type = std::decay_t<decltype(v)>::value_type;

      const auto size = v.size();
      this->write_bits<length_prefix_serialized_type>(sizeof(length_prefix_serialized_type) * 8, size);

      if (!std::is_constant_evaluated()) {
         if (this->is_byte_aligned() && size + this->get_bytepos() <= this->size()) {
            memcpy(this->_buffer + this->get_bytepos(), v.data(), size * sizeof(char_type));
            this->_position.advance_by_bytes(size * sizeof(char_type));
            //
            return;
         }
      }
      /*// IntelliSense chokes and dies on range-based for loops used on std::string and friends during compile-time evaluation.
      for (auto& c : v)
         this->write(c);
      //*/
      for (size_t i = 0; i < v.size(); ++i)
         this->write(v[i]);
   }

   template<typename T>
   constexpr void bitwriter::write(const std::vector<T>& v) {
      this->write_bits<length_prefix_serialized_type>(sizeof(length_prefix_serialized_type) * 8, v.size());
      //
      /*// IntelliSense chokes and dies on range-based for loops used on std::string and friends during compile-time evaluation.
      for (auto& item : v)
         this->write(item);
      //*/
      for (size_t i = 0; i < v.size(); ++i)
         this->write(v[i]);
   }

   template<typename T> requires (std::is_integral_v<T> && !std::is_same_v<T, bool> && !std::is_enum_v<T>)
   constexpr void bitwriter::write_bits(size_t bitcount, T value) {
      this->_ensure_room_for(bitcount);

      // Types like char (as in const char*) are actually signed, and bitshifting them 
      // to the right will extend the sign bit. We really, really don't want that.
      using unsigned_t = std::make_unsigned_t<
         std::conditional_t<
            std::is_integral_v<T> && !std::is_same_v<T, bool>,
            T,
            std::conditional_t<
               std::is_same_v<T, bool>,
               uint8_t,
               void
            >
         >
      >;
      unsigned_t to_write = std::bit_cast<unsigned_t>(value);

      // For signed values, the extra bits will always be 1 (i.e. sign-extended), so we 
      // need to shear those off for the write to work properly.
      to_write &= cobb::bitmax(bitcount);

      while (bitcount > 0) {
         uint8_t& target = this->_access_byte(this->get_bytepos());
         int shift = this->get_bitshift();
         //
         if (!shift) {
            if (bitcount < 8) {
               target = to_write << (8 - bitcount);
               this->_position.advance_by_bits(bitcount);
               return;
            }
            target = (to_write >> (bitcount - 8));
            this->_position.advance_by_bytes(1);
            bitcount -= 8;
            continue;
         }
         int extra = 8 - shift;
         if (bitcount <= extra) {
            target |= (to_write << (extra - bitcount));
            this->_position.advance_by_bits(bitcount);
            return;
         }
         target &= ~(uint8_t(0xFF) >> shift); // clear the bits we're about to write
         target |= ((to_write >> (bitcount - extra)) & 0xFF);
         this->_position.advance_by_bits(extra);
         bitcount -= extra;
      }
   }

   constexpr void bitwriter::write_bitstream(const bitwriter& other) {
      auto* other_data  = other.data();
      auto  other_bytes = other.get_bytepos();
      for (size_t i = 0; i < other_bytes; ++i) {
         this->write_bits(8, other_data[i]);
      }
      if (auto other_bits = other.get_bitshift()) {
         this->write_bits(other_bits, other_data[other_bytes]);
      }
   }
}
