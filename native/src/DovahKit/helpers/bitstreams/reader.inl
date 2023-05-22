#pragma once
#include "./reader.h"
#include "../uint_of_size.h"
#include "./exceptions/missing_data_header.h"
#include "./exceptions/read_past_end.h"

namespace cobb::bitstreams {
   constexpr reader::reader(buffer_type b, size_type size_in_bytes) : _buffer(b), _size(size_in_bytes) {
      if (b)
         this->_read_header();
   }

   constexpr void reader::_consume_byte(uint8_t& out, size_t bitcount, int& consumed) {
      auto    shift = this->get_bitshift();
      uint8_t byte  = this->_buffer[this->get_bytepos()] & (0xFF >> shift);
      int     bits_read = 8 - shift;
      if (bitcount < bits_read) {
         byte = byte >> (bits_read - bitcount);
         this->_position.advance_by_bits(bitcount);
         consumed = bitcount;
      } else {
         this->_position.advance_by_bits(bits_read);
         consumed = bits_read;
      }
      out = byte;
   }
   constexpr void reader::_read_header() {
      this->_header = {};
      this->_header.stream(*this);
   }

   constexpr size_t reader::bits_remaining() const noexcept {
      size_t bytes = this->bytes_remaining();
      size_t shift = this->get_bitshift();
      if (shift) {
         return (bytes * 8) + (8 - shift);
      }
      return bytes * 8;
   }

   constexpr bool reader::is_in_bounds(size_t bytes) const noexcept {
      if (bytes)
         return bytes <= this->bytes_remaining();
      return !this->is_at_end();
   }

   constexpr void reader::require_remaining_bits(size_t b) const {
      if (b > this->bits_remaining())
         throw exceptions::read_past_end{this->_position, b};
   }
   constexpr void reader::require_remaining_bytes(size_t b) const {
      if (b > this->bytes_remaining())
         throw exceptions::read_past_end{ this->_position, b * 8};
   }

   constexpr void reader::set_buffer(buffer_type b, size_t size) noexcept {
      this->_buffer   = b;
      this->_size     = size;
      this->_position = {};

      if (b) {
         this->_read_header();
      }
   }
   constexpr void reader::set_bitpos(size_t bitpos) {
      if (bitpos > this->_size * 8)
         throw exceptions::read_past_end{ this->_position, bitpos - (this->_size * 8) };
      this->_position.set_in_bits(bitpos);
   }
   constexpr void reader::set_bytepos(size_t bytepos) {
      if (bytepos > this->_size)
         throw exceptions::read_past_end{ this->_position, (bytepos - this->_size) * 8 };
      this->_position.set_in_bytes(bytepos);
   }

   #pragma region Stream overloads
   template<bitstreamable_primitive T> requires (!std::is_const_v<T>)
   constexpr void reader::stream(T& v) {
      constexpr const size_t bitcount = std::is_same_v<T, bool> ? 1 : sizeof(T) * 8;
      this->require_remaining_bits(bitcount);

      this->unchecked_stream(v);
   }
   //
   template<bitstreamable_primitive T> requires (!std::is_const_v<T>)
   constexpr void reader::unchecked_stream(T& v) {
      constexpr const size_t bitcount = std::is_same_v<T, bool> ? 1 : sizeof(T) * 8;

      if constexpr (std::is_floating_point_v<T>) {
         using bit_castable_type = uint_of_size<sizeof(T)>;

         v = std::bit_cast<T>((bit_castable_type)this->stream_bits(bitcount));
      } else {
         v = (T)this->stream_bits(bitcount);
      }
   }
   
   template<impl::_override_bitcount::readable_specialization Wrapper>
   constexpr void reader::unchecked_stream(const Wrapper& wrapper) {
      using value_type = typename Wrapper::value_type;

      wrapper.target = (value_type)this->stream_bits(Wrapper::bitcount);
   }

   template<size_t length_bitcount, bitstreamable_container T> requires (!std::is_same_v<T, QString>)
   constexpr void reader::stream(T& v) {
      this->require_remaining_bits(length_bitcount);

      size_t size = this->unchecked_stream_bits(length_bitcount);
      v.resize(size);

      if (!size)
         return;

      if constexpr (bitstreamable_primitive<typename T::value_type>) {

         constexpr const size_t bitcount_per_item = std::is_same_v<typename T::value_type, bool> ? 1 : sizeof(typename T::value_type) * 8;
         //
         if (bitcount_per_item * size > this->bits_remaining()) {
            this->_position.rewind_by_bits(length_bitcount);
            throw exceptions::read_past_end{};
         }

         if constexpr (std::is_trivially_copyable_v<typename T::value_type> && !std::is_same_v<typename T::value_type, bool>) {
            //
            // If this is a type we can just blindly copy, and if we're byte-aligned and 
            // not being run at compile-time, then let's save time with a simple memcpy.
            //
            if (!std::is_constant_evaluated()) {
               if (this->is_byte_aligned()) {
                  const size_t bytecount = sizeof(typename T::value_type) * size;

                  memcpy(v.data(), this->_buffer + this->get_bytepos(), bytecount);
                  this->_position.advance_by_bytes(bytecount);
                  return;
               }
            }
         }

         for (size_t i = 0; i < size; ++i) {
            //
            // Unchecked, since we already bounds-checked for the whole container, above.
            //
            this->unchecked_stream(v[i]);
         }
      } else {
         for (size_t i = 0; i < size; ++i) {
            this->stream(v[i]);
         }
      }
   }

   constexpr void reader::stream(cobb::eight_cc& v) {
      this->require_remaining_bytes(8);
      this->unchecked_stream(v);
   }
   constexpr void reader::unchecked_stream(cobb::eight_cc& v) {
      for (size_t i = 0; i < 8; ++i)
         this->unchecked_stream(v.bytes[i]);
   }
   #pragma endregion

   constexpr uintmax_t reader::stream_bits(size_t bitcount) {
      this->require_remaining_bits(bitcount);
      return this->unchecked_stream_bits(bitcount);
   }
   constexpr uintmax_t reader::unchecked_stream_bits(size_t bitcount) {
      uintmax_t result = 0;
      uint8_t   bits;
      int       consumed;
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

   template<bitstreamable_primitive T>
   constexpr void reader::stream_bits(size_t bitcount, T& v) {
      this->require_remaining_bits(bitcount);
      this->unchecked_stream_bits(bitcount, v);
   }
   template<bitstreamable_primitive T>
   constexpr void reader::unchecked_stream_bits(size_t bitcount, T& v) {
      uintmax_t result = 0;
      uint8_t   bits;
      int       consumed;
      this->_consume_byte(bits, bitcount, consumed);
      result = bits;
      int remaining = bitcount - consumed;
      while (remaining) {
         this->_consume_byte(bits, remaining, consumed);
         result = (result << consumed) | bits;
         remaining -= consumed;
      }

      if constexpr (std::is_floating_point_v<T>) {
         using bit_castable_type = uint_of_size<sizeof(T)>;

         v = std::bit_cast<T>((bit_castable_type)result);
      } else {
         v = (T)result;
      }
   }
}