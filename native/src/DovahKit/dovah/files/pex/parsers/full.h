#pragma once
#include <bit>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <vector>
#include "./_util.h"

#include "../file_header.h"
#include "../function_debug_info.h"
#include "../script_object.h"
#include "../user_flag_definition.h"

namespace dovah::pex {
   struct compiled_function;
   struct compiled_opcode;
   struct function_debug_info;
   struct property_declaration;
   struct state;
   struct variable_declaration;
   struct value;
}

namespace dovah::pex::parsers {
   class full {
      public:
         struct debug_info {
            uint64_t modification_time = 0;
            std::vector<function_debug_info> per_function;
         };

      protected:
         struct {
            const uint8_t* _buffer = nullptr;
            uint32_t       _pos    = 0;
            size_t         _size   = 0;
         } file;
         bool _needs_endian_swap = false;
         std::vector<std::string> _string_table;

         constexpr void _require_remaining_bytes(size_t);
         constexpr void _set_position(size_t s);

      public:
         constexpr const std::string& get_tabled_string(uint16_t index) const;

         #pragma region read
         template<typename... Types> requires (sizeof...(Types) > 1)
         constexpr void read(Types&... args) {
            (read(args), ...);
         }

         template<typename T> requires util::supports_stream<T, full>
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
         file_header header;
         std::optional<debug_info> debug;
         std::vector<user_flag_definition> user_flags;
         std::vector<script_object> objects;

         constexpr size_t get_position() const { return this->file._pos; }
         
         constexpr void read_file(const uint8_t* buffer, size_t size);

         constexpr const script_object* lookup_object(const std::string&) const;
   };
   static_assert(util::is_valid_stream_class<full>);
}

#include "./full.inl"