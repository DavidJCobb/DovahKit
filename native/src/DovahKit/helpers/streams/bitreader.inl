#pragma once
#include "./bitreader.h"

namespace cobb::streams {
   constexpr void bitreader::_advance_offset_by_bits(size_t bits) {
      this->_position.advance_by_bits(bits);
      this->_position.clamp_to_size(this->_size);
   }
   constexpr void bitreader::_advance_offset_by_bytes(size_t bytes) {
      this->_position.advance_by_bytes(bytes);
      this->_position.clamp_to_size(this->_size);
   }
   constexpr void bitreader::_byte_align() {
      this->_position.advance_to_next_byte();
      this->_position.clamp_to_size(this->_size);
   }
   constexpr void bitreader::_consume_byte(uint8_t& out, uint8_t bitcount, int& consumed) {
      auto bytepos = this->get_bytepos();
      if (bytepos >= this->_size) {
         out = 0;
         this->_position.add_overshoot_bits(bitcount);
         consumed = bitcount;
         return;
      }
      auto    shift = this->get_bitshift();
      uint8_t byte  = this->_buffer[bytepos] & (0xFF >> shift);
      int     bits_read = 8 - shift;
      if (bitcount < bits_read) {
         byte = byte >> (bits_read - bitcount);
         this->_advance_offset_by_bits(bitcount);
         consumed = bitcount;
      } else {
         this->_advance_offset_by_bits(bits_read);
         consumed = bits_read;
      }
      out = byte;
   }
   constexpr uint64_t bitreader::_read_bits(uint8_t bitcount) {
      uint64_t result = 0;
      uint8_t  bits;
      int      consumed;
      this->_consume_byte(bits, bitcount, consumed);
      result = bits;
      int remaining = bitcount - consumed;
      while (remaining) {
         this->_consume_byte(bits, remaining, consumed);
         result = (result << consumed) | bits;
         remaining -= consumed;
      }
      return result;
   }

   constexpr bool bitreader::is_in_bounds(size_t bytes) const noexcept {
      if (bytes)
         return this->get_bytepos() + bytes <= this->size();
      return !this->is_at_end();
   }

   constexpr void bitreader::set_buffer(buffer_type b, size_t size) {
      this->_buffer   = b;
      this->_size     = size;
      this->_position = {};
   }
   constexpr void bitreader::set_bitpos(size_t bitpos) {
      this->_position.set_in_bits(bitpos);
      if (this->_position.bytes > this->size()) {
         this->_position.bytes = this->size();
         this->_position.bits  = 0;
         this->_position.set_overshoot_bits(bitpos - (this->size() * 8));
      } else
         this->_position.overshoot = {};
   }
   constexpr void bitreader::set_bytepos(size_t bytepos) {
      this->_position.set_in_bytes(bytepos);
      if (bytepos > this->size()) {
         this->_position.bytes = this->size();
         this->_position.set_overshoot_bytes(bytepos - this->size());
      } else
         this->_position.overshoot = {};
   }

   constexpr void bitreader::read(std::string& v) {
      using char_type = std::decay_t<decltype(v)>::value_type;

      length_prefix_serialized_type size = this->read_bits<length_prefix_serialized_type>(sizeof(length_prefix_serialized_type) * 8);
      v.resize(size);
      if (size == 0)
         return;

      if (!std::is_constant_evaluated()) {
         if (this->is_byte_aligned() && size + this->get_bytepos() <= this->size()) {
            memcpy(v.data(), this->_buffer + this->get_bytepos(), size * sizeof(char_type));
            this->_position.advance_by_bytes(size * sizeof(char_type));
            //
            return;
         }
      }
      /*// IntelliSense chokes and dies on range-based for loops used on std::string and friends during compile-time evaluation.
      for (auto& c : v)
         this->read(c);
      //*/
      for (size_t i = 0; i < v.size(); ++i)
         this->read(v[i]);
   }
   constexpr void bitreader::read(std::wstring& v) {
      using char_type = std::decay_t<decltype(v)>::value_type;

      length_prefix_serialized_type size = this->read_bits<length_prefix_serialized_type>(sizeof(length_prefix_serialized_type) * 8);
      v.resize(size);
      if (size == 0)
         return;

      if (!std::is_constant_evaluated()) {
         if (this->is_byte_aligned() && size + this->get_bytepos() <= this->size()) {
            memcpy(v.data(), this->_buffer + this->get_bytepos(), size * sizeof(char_type));
            this->_position.advance_by_bytes(size * sizeof(char_type));
            //
            return;
         }
      }
      /*// IntelliSense chokes and dies on range-based for loops used on std::string and friends during compile-time evaluation.
      for (auto& c : v)
         this->read(c);
      //*/
      for (size_t i = 0; i < v.size(); ++i)
         this->read(v[i]);
   }

   template<typename T>
   constexpr void bitreader::read(std::vector<T>& v) {
      length_prefix_serialized_type size = this->read_bits<length_prefix_serialized_type>(sizeof(length_prefix_serialized_type) * 8);
      v.resize(size);
      if (size == 0)
         return;

      /*// IntelliSense chokes and dies on range-based for loops used on std::string and friends during compile-time evaluation.
      for (auto& item : v)
         this->read(item);
      //*/
      for (size_t i = 0; i < v.size(); ++i)
         this->read(v[i]);
   }

   template<typename T> requires (std::is_integral_v<T> && !std::is_same_v<T, bool> && !std::is_enum_v<T>)
   constexpr T bitreader::read_bits(uint8_t bitcount) {
      using uT = std::make_unsigned_t<cobb::strip_enum_t<T>>;
      //
      uT result = this->_read_bits(bitcount);
      return (T)result;
   }
}