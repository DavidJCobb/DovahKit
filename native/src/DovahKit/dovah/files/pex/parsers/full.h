#pragma once
#include <bit>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <vector>
#include "./_util.h"
#include "./base.h"

#include "../file_header.h"
#include "../function_debug_info.h"
#include "../script_object.h"
#include "../user_flag_definition.h"

namespace dovah::pex::parsers {
   class full;
   class full : public base<full> {
      public:
         struct debug_info {
            uint64_t modification_time = 0;
            std::vector<function_debug_info> per_function;
         };

      protected:
         std::vector<std::string> _string_table;

      public:
         constexpr const std::string_view get_tabled_string(uint16_t index) const;
         
      public:
         file_header header;
         std::optional<debug_info> debug;
         std::vector<user_flag_definition> user_flags;
         std::vector<script_object> objects;

         constexpr void read_file(const uint8_t* buffer, size_t size);

         constexpr const script_object* lookup_object(const std::string&) const;
   };
   static_assert(util::is_valid_stream_class<full>);
}

#include "./full.inl"