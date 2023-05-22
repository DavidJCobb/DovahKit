#pragma once
#include "./writer.h"
#include <stdexcept>
#include "../uint_of_size.h"
#include "./exceptions/container_length_too_large.h"

namespace cobb::bitstreams {
   constexpr writer::~writer() {
      this->_buffer_free();
      this->_size     = 0;
      this->_position = {};
   }
   
   constexpr void writer::_buffer_free() {
      if (!this->_buffer)
         return;
      if (std::is_constant_evaluated()) {
         delete[] this->_buffer;
      } else {
         free(this->_buffer);
      }
      this->_buffer = nullptr;
   }

   constexpr uint8_t& writer::_access_byte(size_t bytepos) const noexcept {
      return this->_buffer[bytepos];
   }
   constexpr void writer::_ensure_room_for(unsigned int bitcount) {
      size_t bitsize = (size_t)this->_size * 8;
      size_t target  = (size_t)this->get_bitpos() + bitcount;
      if (target > bitsize) {
         target += 8 - (target % 8);
         this->resize(target / 8);
      }
   }

   constexpr void writer::reserve(size_t size) {
      if (size < this->_size)
         return;
      this->resize(size);
   }
   constexpr void writer::resize(size_t size) {
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
               throw std::bad_alloc{};
            }
         }
      }
      this->_size = size;
      if (size <= this->_position.bytes) {
         this->_position.bytes = size;
         this->_position.bits  = 0;
      }
   }

   constexpr void writer::skip_bits(size_t b) {
      size_t total = this->get_bitpos() + b;
      this->_ensure_room_for(total);
      this->_position.bytes = total / 8;
      this->_position.bits  = total % 8;
   }

   #pragma region Stream overloads
   template<bitstreamable_primitive T>
   constexpr void writer::unchecked_stream(const T& v) {
      constexpr const size_t bitcount = std::is_same_v<T, bool> ? 1 : sizeof(T) * 8;

      if constexpr (std::is_floating_point_v<T>) {
         using bit_castable_type = uint_of_size<sizeof(T)>;

         this->stream_bits(bitcount, std::bit_cast<bit_castable_type>(v));
      } else {
         this->stream_bits(bitcount, v);
      }
   }

   template<impl::_override_bitcount::writable_specialization Wrapper>
   constexpr void writer::unchecked_stream(const Wrapper& wrapper) {
      using value_type = typename Wrapper::value_type;

      constexpr const size_t bitcount = Wrapper::bitcount;
      const auto& v = wrapper.target;

      this->stream_bits(bitcount, v);
   }

   template<size_t length_bitcount, bitstreamable_container T> requires (!std::is_same_v<T, QString>)
   constexpr void writer::stream(const T& v) {
      size_t size = v.size();
      if constexpr (length_bitcount < sizeof(uintmax_t) / 8) {
         if (size > (uintmax_t(1) << length_bitcount - 1)) {
            throw exceptions::container_length_too_large{};
         }
      }
      this->stream_bits(length_bitcount, size);
      if (!size)
         return;

      if constexpr (bitstreamable_primitive<typename T::value_type>) {
         if constexpr (std::is_trivially_copyable_v<typename T::value_type> && !std::is_same_v<typename T::value_type, bool>) {
            //
            // If this is a type we can just blindly copy, and if we're byte-aligned and 
            // not being run at compile-time, then let's save time with a simple memcpy.
            //
            if (!std::is_constant_evaluated()) {
               if (this->is_byte_aligned()) {
                  const size_t bytecount = sizeof(typename T::value_type) * size;

                  memcpy(this->_buffer + this->get_bytepos(), v.data(), bytecount);
                  this->_position.advance_by_bytes(bytecount);
                  return;
               }
            }
         }
      }
      for (size_t i = 0; i < size; ++i) {
         this->stream(v[i]);
      }
   }

   constexpr void writer::unchecked_stream(const cobb::eight_cc& v) {
      for (size_t i = 0; i < 8; ++i)
         this->unchecked_stream(v.bytes[i]);
   }
   #pragma endregion

   template<bitstreamable_primitive T>
   constexpr void writer::unchecked_stream_bits(size_t bitcount, const T& v) {
      this->_ensure_room_for(bitcount);

      // Types like char (as in const char*) are actually signed, and bitshifting them 
      // to the right will extend the sign bit. We really, really don't want that.
      using unsigned_value_type = std::conditional_t<
         std::is_same_v<T, bool>,
         uint8_t,
         uint_of_size<sizeof(T)>
      >;
      unsigned_value_type to_write = std::bit_cast<unsigned_value_type>(v);

      // For signed values, the extra bits will always be 1 (i.e. sign-extended), so we 
      // need to shear those off for the write to work properly.
      if (bitcount < sizeof(unsigned_value_type) * 8) {
         auto mask = (unsigned_value_type(1) << bitcount) - 1;
         to_write &= mask;
      }

      while (bitcount > 0) {
         uint8_t& target = this->_buffer[this->get_bytepos()];
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
}