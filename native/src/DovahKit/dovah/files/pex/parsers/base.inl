#pragma once
#include "./base.h"

#include "helpers/uint_of_size.h"

#include "./exceptions/unexpected_eof.h"


#define CLASS_TEMPLATE_PARAMS template<typename Self, typename BufferItemType> requires (sizeof(BufferItemType) == 1)
#define CLASS_NAME base<Self, BufferItemType>

namespace dovah::pex::parsers {
   CLASS_TEMPLATE_PARAMS
   constexpr void CLASS_NAME::_require_remaining_bytes(size_t count) {
      if (this->get_position() + count > this->file._size)
         throw exceptions::unexpected_eof(*this);
   }

   CLASS_TEMPLATE_PARAMS
   constexpr void CLASS_NAME::_set_position(size_t s) {
      if (s > this->file._size) {
         throw exceptions::unexpected_eof(*this);
      }
      this->file._pos = s;
   }

   #pragma region read
   CLASS_TEMPLATE_PARAMS
   template<typename T> requires util::supports_stream<T, Self>
   constexpr void CLASS_NAME::read(T& v) {
      v.read(*(Self*)this);
   }

   CLASS_TEMPLATE_PARAMS
   template<typename T> requires std::is_floating_point_v<T>
   constexpr void CLASS_NAME::read(T& v) {
      using castable = cobb::uint_of_size<sizeof(T)>;

      castable bits;
      read(bits);
      v = std::bit_cast<T, castable>(bits);
   }

   CLASS_TEMPLATE_PARAMS
   template<typename T> requires std::is_enum_v<T>
   constexpr void CLASS_NAME::read(T& v) {
      using castable = cobb::uint_of_size<sizeof(T)>;

      castable bits;
      read(bits);
      v = std::bit_cast<T, castable>(bits);
   }

   CLASS_TEMPLATE_PARAMS
   template<typename T> requires std::is_integral_v<T>
   constexpr void CLASS_NAME::read(T& v) {
      this->_require_remaining_bytes(sizeof(T));
      if (std::is_constant_evaluated()) {
         if constexpr (sizeof(T) > 1) {
            v = 0;
            for (size_t i = 0; i < sizeof(T); ++i) {
               v |= (T)this->file._buffer[this->file._pos + i] << (0x08 * i);
            }
         } else {
            v = (T)this->file._buffer[this->file._pos];
         }
      } else {
         v = *(const T*)(this->file._buffer + this->get_position());
      }
      if constexpr (sizeof(T) > 1) {
         if (this->_needs_endian_swap) {
            v = std::byteswap(v);
         }
      }
      this->file._pos += sizeof(T);
   }

   CLASS_TEMPLATE_PARAMS
   template<size_t PrefixBytecount>
   constexpr void CLASS_NAME::read_length_prefixed_string(std::string& v) {
      v.clear();

      cobb::uint_of_size<PrefixBytecount> count = 0;
      read(count);
      this->_require_remaining_bytes(count);
      v.resize(count);
      if (std::is_constant_evaluated()) {
         for (auto& c : v)
            read(c);
      } else {
         memcpy(v.data(), this->file._buffer + this->get_position(), count);
         this->file._pos += count;
      }
   }

   CLASS_TEMPLATE_PARAMS
   template<size_t PrefixBytecount, typename T>
   constexpr void CLASS_NAME::read_length_prefixed_vector(std::vector<T>& v) {
      v.clear();

      cobb::uint_of_size<PrefixBytecount> count = 0;
      read(count);
      if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T>) {
         size_t bytes_to_read = sizeof(T) * count;

         this->_require_remaining_bytes(bytes_to_read);
         v.resize(count);
         if (std::is_constant_evaluated()) {
            for (auto& item : v)
               read(item);
         } else {
            memcpy(v.data(), (const char*)this->file._buffer + this->get_position(), bytes_to_read);
            this->file._pos += bytes_to_read;
         }
      } else {
         //
         // Can't know in advance how many bytes a struct will need to read.
         //
         for (size_t i = 0; i < count; ++i) {
            read(v.emplace_back());
         }
      }
   }
   #pragma endregion

   #pragma region skip
   CLASS_TEMPLATE_PARAMS
   template<size_t PrefixBytecount>
   constexpr void CLASS_NAME::skip_length_prefixed_string() {
      cobb::uint_of_size<PrefixBytecount> count = 0;
      read(count);
      this->skip_bytes(count);
   }

   CLASS_TEMPLATE_PARAMS
   template<size_t PrefixBytecount, typename ValueType>
   constexpr void CLASS_NAME::skip_length_prefixed_vector() {
      cobb::uint_of_size<PrefixBytecount> count = 0;
      read(count);
      if constexpr (std::is_integral_v<ValueType> || std::is_floating_point_v<ValueType>) {
         this->skip_bytes(sizeof(ValueType) * count);
      } else if constexpr (util::supports_stream<ValueType, Self>) {
         for (size_t i = 0; i < count; ++i)
            ValueType::skip(*(Self*)this);
      }
   }
   #pragma endregion
}

#undef CLASS_TEMPLATE_PARAMS
#undef CLASS_NAME