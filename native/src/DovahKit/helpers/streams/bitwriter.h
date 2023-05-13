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

namespace cobb::streams {
   class bitwriter;

   namespace impl::_bitwriter {
      template<typename T> concept bitwritable = requires(const T& x, bitwriter& stream) {
         { x.write(stream) };
      };
   }

   class bitwriter {
      public:
         using position_type = bitstream_position;

         using buffer_type = std::uint8_t*;
         using size_type   = size_t;

         using length_prefix_serialized_type = std::uint32_t;
         
      protected:
         template<typename T> static constexpr const size_t _bitcount_of_type = std::bit_width(std::numeric_limits<std::make_unsigned_t<cobb::strip_enum_t<T>>>::max());

      protected:
         buffer_type   _buffer   = nullptr; // owned
         size_type     _size     = 0; // in bytes
         position_type _position = {};

      protected:
         constexpr void _buffer_free();

         constexpr uint8_t& _access_byte(size_t bytepos) const noexcept;
         constexpr void _ensure_room_for(unsigned int bitcount);

      public:
         constexpr bitwriter() {}
         constexpr ~bitwriter();

         constexpr buffer_type data() const noexcept { return this->_buffer; }
         constexpr size_type size() const noexcept { return this->_size; }

         constexpr position_type get_position() const noexcept { return this->_position; }
         //
         constexpr size_t get_bitpos() const noexcept { return this->_position.in_bits(); }
         constexpr size_t get_bitshift() const noexcept { return this->_position.bits; }
         constexpr size_t get_bytepos() const noexcept { return this->_position.bytes; }
         constexpr size_t get_bytespan() const noexcept { return this->_position.bytespan(); }

         constexpr bool is_byte_aligned() const noexcept { return this->get_bitshift() == 0; }

         constexpr void enlarge_by(size_t bytes) { this->resize(this->size() + bytes); }
         constexpr void reserve(size_t bytes);
         constexpr void resize(size_t bytes);

         // Multi-write call
         template<typename... Types> requires (sizeof...(Types) > 1)
         constexpr void write(const Types&... args) {
            (this->write(args), ...);
         }

         #pragma region write
         template<typename T> constexpr void write(const T& v) = delete;

         template<impl::_bitwriter::bitwritable T>
         constexpr void write(const T& value) {
            value.write(*this);
         }

         template<typename T> requires (!std::is_const_v<T>&& std::is_arithmetic_v<T> && !std::is_same_v<T, bool> && !std::is_floating_point_v<T>)
         constexpr void write(const T v) {
            this->write_bits<T>(_bitcount_of_type<T>, v);
         }

         template<typename T> requires (!std::is_const_v<T>&& std::is_enum_v<T>)
         constexpr void write(const T v) {
            constexpr const size_t bc = bitcount_of_enum<T>;
            
            this->write_bits<cobb::strip_enum_t<T>>(bc, (cobb::strip_enum_t<T>)v);
         }

         constexpr void write(const bool v) {
            this->write_bits(1, (uint8_t)v);
         }
         constexpr void write(const double v) {
            this->write_bits(64, std::bit_cast<std::uint64_t>(v));
         }
         constexpr void write(const float v) {
            this->write_bits(32, std::bit_cast<std::uint32_t>(v));
         }

         constexpr void write(cobb::eight_cc& v) {
            for (size_t i = 0; i < 8; ++i)
               this->write(v.bytes[i]);
         }

         constexpr void write(const std::string& v);
         constexpr void write(const std::wstring& v);

         template<typename T>
         constexpr void write(const std::vector<T>& v);
         #pragma endregion

         template<typename T> requires (std::is_integral_v<T> && !std::is_same_v<T, bool> && !std::is_enum_v<T>)
         constexpr void write_bits(size_t bitcount, T value);

         constexpr void write_bitstream(const bitwriter&);
   };
}

#include "./bitwriter.inl"