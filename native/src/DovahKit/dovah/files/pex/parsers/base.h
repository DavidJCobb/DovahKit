#pragma once
#include <bit>
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <vector>
#include "./_util.h"

namespace dovah::pex::parsers {
   template<typename Self, typename BufferItemType = uint8_t> requires (sizeof(BufferItemType) == 1)
   class base {
      protected:
         struct {
            const BufferItemType* _buffer = nullptr;
            size_t   _pos    = 0;
            size_t   _size   = 0;
         } file;
         bool _needs_endian_swap = false;

         constexpr void _require_remaining_bytes(size_t);
         constexpr void _set_position(size_t s);

      public:
         #pragma region read
         template<typename... Types> requires (sizeof...(Types) > 1)
         constexpr void read(Types&... args) {
            (read(args), ...);
         }

         template<typename T> requires util::supports_stream<T, Self>
         constexpr void read(T& v);
         
         template<typename T> requires std::is_floating_point_v<T>
         constexpr void read(T& v);

         template<typename T> requires std::is_enum_v<T>
         constexpr void read(T& v);

         template<typename T> requires std::is_integral_v<T>
         constexpr void read(T& v);

         template<size_t PrefixBytecount>
         constexpr void read_length_prefixed_string(std::string& v);

         template<size_t PrefixBytecount, typename T>
         constexpr void read_length_prefixed_vector(std::vector<T>& v);
         #pragma endregion

         #pragma region skip
         constexpr void skip_bytes(size_t s) {
            _set_position(get_position() + s);
         }

         template<size_t PrefixBytecount>
         constexpr void skip_length_prefixed_string();

         template<size_t PrefixBytecount, typename ValueType>
         constexpr void skip_length_prefixed_vector();
         #pragma endregion
         
      public:
         constexpr void set_buffer(const BufferItemType* data, size_t size) {
            this->file = {
               ._buffer = data,
               ._pos    = 0,
               ._size   = size,
            };
         }

         constexpr const void* get_buffer_data() const { return (const void*)this->file._buffer; }
         constexpr size_t get_buffer_size() const { return this->file._size; }
         constexpr size_t get_position() const { return this->file._pos; }

         constexpr size_t bytes_remaining() const { return this->file._size - get_position(); }
   };
}

#include "./base.inl"