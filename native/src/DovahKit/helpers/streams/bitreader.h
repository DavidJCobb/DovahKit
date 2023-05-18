#pragma once
#include <bit>
#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>
#include "../type_traits/strip_enum.h"
#include "../eight_cc.h"
#include "./bitcount_of_enum.h"
#include "./bitstream_position.h"

class QString;

namespace cobb::streams {
   class bitreader;

   namespace impl::_bitreader {
      template<typename T> concept bitreadable = requires(T& x, bitreader& stream) {
         { x.read(stream) };
      };
   }

   class bitreader {
      public:
         using basic_position_type = bitstream_position;
         using position_type       = full_bitstream_position;

         using buffer_type = const std::uint8_t*;
         using size_type   = size_t;

         using length_prefix_serialized_type = std::uint32_t;

      protected:
         template<typename T> static constexpr size_t _bitcount_of_type = std::bit_width(std::numeric_limits<std::make_unsigned_t<cobb::strip_enum_t<T>>>::max());

      protected:
         buffer_type   _buffer   = nullptr; // unowned
         size_type     _size     = 0; // in bytes
         position_type _position = {};

      protected:
         constexpr void _advance_offset_by_bits(size_t bits);
         constexpr void _advance_offset_by_bytes(size_t bytes);
         constexpr void _byte_align();
         constexpr void _consume_byte(uint8_t& out, uint8_t bitcount, int& consumed); // reads {std::min(std::min(8, bitcount), (8 - this->_position.bits))} bits from the buffer
         constexpr uint64_t _read_bits(uint8_t bitcount);

      public:
         constexpr bitreader() {}

         constexpr buffer_type data() const noexcept { return this->_buffer; }
         constexpr size_type size() const noexcept { return this->_size; }

         constexpr basic_position_type get_position() const noexcept { return this->_position; }
         constexpr position_type get_full_position() const noexcept { return this->_position; }
         //
         constexpr size_t get_bitpos() const noexcept { return this->_position.in_bits(); }
         constexpr size_t get_bitshift() const noexcept { return this->_position.bits; }
         constexpr size_t get_bytepos() const noexcept { return this->_position.bytes; }
         constexpr size_t get_bytespan() const noexcept { return this->_position.bytespan(); }
         constexpr size_t get_overshoot_bits() const noexcept { return this->_position.overshoot_in_bits(); }
         constexpr size_t get_overshoot_bytes() const noexcept { return this->_position.overshoot_in_bytes(); }

         constexpr bool is_at_end() const { return this->get_bytepos() >= this->size(); }
         constexpr bool is_in_bounds(size_t bytes = 0) const noexcept;
         constexpr bool is_byte_aligned() const noexcept { return this->get_bitshift() == 0; }

         constexpr void set_buffer(buffer_type, size_t size_in_bytes);
         constexpr void set_bitpos(size_t b);
         constexpr void set_bytepos(size_t b);

         // Multi-read call
         template<typename... Types> requires (sizeof...(Types) > 1)
         constexpr void read(Types&... args) {
            (this->read(args), ...);
         }

         #pragma region read
         template<impl::_bitreader::bitreadable T>
         constexpr void read(T& v) {
            v.read(*this);
         }

         template<typename T> requires (!std::is_const_v<T> && std::is_arithmetic_v<T> && !std::is_same_v<T, bool> && !std::is_floating_point_v<T>)
         constexpr void read(T& v) {
            v = this->read_bits<T>(_bitcount_of_type<T>);
         }

         template<typename T> requires (!std::is_const_v<T> && std::is_enum_v<T>)
         constexpr void read(T& v) {
            constexpr const size_t bc = bitcount_of_enum<T>;

            v = (T)this->read_bits<cobb::strip_enum_t<T>>(bc);
         }

         constexpr void read(bool& v) {
            v = (bool)this->read_bits<uint8_t>(1);
         }
         constexpr void read(double& v) {
            v = std::bit_cast<double>(this->read_bits<std::uint64_t>(64));
         }
         constexpr void read(float& v) {
            v = std::bit_cast<float>(this->read_bits<std::uint32_t>(32));
         }

         constexpr void read(cobb::eight_cc& v) {
            for (size_t i = 0; i < 8; ++i)
               this->read(v.bytes[i]);
         }

         constexpr void read(std::string& v);
         constexpr void read(std::wstring& v);

         template<typename T>
         constexpr void read(std::vector<T>& v);

         void read(QString&);
         #pragma endregion

         constexpr void skip_bits(size_t b) { this->_advance_offset_by_bits(b); }
         constexpr void skip_bytes(size_t b) { this->_advance_offset_by_bytes(b); }

         template<typename T = uint32_t> requires (std::is_integral_v<T> && !std::is_same_v<T, bool> && !std::is_enum_v<T>)
         constexpr T read_bits(uint8_t bitcount);
   };
}

#include "./bitreader.inl"