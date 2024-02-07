#pragma once
#include "./full.h"

#include "helpers/uint_of_size.h"

#include "dovah/data/papyrus/helpers/name_equals.h"

#include "./exceptions/bad_header_sentinel.h"
#include "./exceptions/bad_string_id.h"
#include "./exceptions/unexpected_eof.h"

namespace dovah::pex::parsers {
   constexpr const std::string& full::get_tabled_string(uint16_t index) const {
      if (index >= this->_string_table.size())
         throw exceptions::bad_string_id(*this, index);
      return this->_string_table[index];
   }

   constexpr void full::_require_remaining_bytes(size_t count) {
      if (this->get_position() + count > this->file._size)
         throw exceptions::unexpected_eof(*this);
   }
   constexpr void full::_set_position(size_t s) {
      if (s > this->file._size) {
         throw exceptions::unexpected_eof(*this);
      }
      this->file._pos = s;
   }

   #pragma region read
   template<typename T> requires util::supports_stream<T, full>
   constexpr void full::read(T& v) {
      v.read(*this);
   }
         
   template<typename T> requires std::is_floating_point_v<T>
   constexpr void full::read(T& v) {
      using castable = cobb::uint_of_size<sizeof(T)>;

      castable bits;
      read(bits);
      v = std::bit_cast<T, castable>(bits);
   }
         
   template<typename T> requires std::is_enum_v<T>
   constexpr void full::read(T& v) {
      using castable = cobb::uint_of_size<sizeof(T)>;

      castable bits;
      read(bits);
      v = std::bit_cast<T, castable>(bits);
   }

   template<typename T> requires std::is_integral_v<T>
   constexpr void full::read(T& v) {
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

   template<size_t PrefixBytecount>
   constexpr void full::read_length_prefixed_string(std::string& v) {
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
   
   template<size_t PrefixBytecount, typename T>
   constexpr void full::read_length_prefixed_vector(std::vector<T>& v) {
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
   template<size_t PrefixBytecount>
   constexpr void full::skip_length_prefixed_string() {
      cobb::uint_of_size<PrefixBytecount> count = 0;
      read(count);
      this->skip_bytes(count);
   }

   template<size_t PrefixBytecount, typename ValueType>
   constexpr void full::skip_length_prefixed_vector() {
      cobb::uint_of_size<PrefixBytecount> count = 0;
      read(count);
      if constexpr (std::is_integral_v<ValueType> || std::is_floating_point_v<ValueType>) {
         this->skip_bytes(sizeof(ValueType) * count);
      } else if constexpr (util::supports_stream<ValueType, full>) {
         for (size_t i = 0; i < count; ++i)
            ValueType::skip(*this);
      }
   }
   #pragma endregion
   
   constexpr void full::read_file(const uint8_t* buffer, size_t size) {
      this->file._pos    = 0;
      this->file._buffer = buffer;
      this->file._size   = size;
      {
         uint32_t magic;
         this->read(magic);
         if (magic != 0xFA57C0DE) {
            if (magic != std::byteswap(0xFA57C0DE))
               throw exceptions::bad_header_sentinel(*this);
            this->_needs_endian_swap = true;
         }
      }
      this->read(header);
      {
         uint16_t count;
         this->read(count);
         this->_string_table.resize(count);
         for (auto& s : this->_string_table)
            this->read_length_prefixed_string<2>(s);
      }
      {
         bool presence;
         this->read(presence);
         if (presence) {
            auto& data = this->debug.emplace();
            this->read(data.modification_time);
            this->read_length_prefixed_vector<2>(data.per_function);
         }
      }
      this->read_length_prefixed_vector<2>(this->user_flags);
      this->read_length_prefixed_vector<2>(this->objects);
   }

   constexpr const script_object* full::lookup_object(const std::string& name) const {
      for (auto it = this->objects.rbegin(); it != this->objects.rend(); ++it) {
         const auto& item = *it;
         if (dovah::papyrus::helpers::name_equals(item.name, name))
            return &item;
      }
      return nullptr;
   }
}