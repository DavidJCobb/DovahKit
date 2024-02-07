#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "./tabled_string.h"

namespace dovah::pex {
   struct function_debug_info {
      public:
         enum class function_type : uint8_t {
            normal = 0,
            getter = 1,
            setter = 2,
            // UESP says that "3" can appear here, too?
         };

      public:
         tabled_string object;
         tabled_string state;
         tabled_string function_name;
         function_type type = function_type::normal;
         //
         std::vector<uint16_t> line_numbers; // maps instructions in the function to their original line numbers in the source code

         template<typename Stream>
         constexpr void read(Stream&);
         //
         template<typename Stream>
         static constexpr void skip(Stream&);
   };
}

#include "./function_debug_info.inl"