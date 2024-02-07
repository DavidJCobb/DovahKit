#pragma once
#include <cstdint>
#include <string>
#include "./compiled_function.h"
#include "./tabled_string.h"

namespace dovah::pex {
   struct property_declaration {
      struct flag {
         flag() = delete;
         enum {
            read    = 0x01,
            write   = 0x02,
            autovar = 0x04,
         };
      };
      
      tabled_string name;
      tabled_string type;
      tabled_string docstring;
      uint32_t      flags          = 0; // user_flags
      uint8_t       property_flags = 0; // property::flag
      tabled_string autovar_name;
      //
      compiled_function getter; // if (read)  flag and no (autovar) flag
      compiled_function setter; // if (write) flag and no (autovar) flag

      template<typename Stream>
      constexpr void read(Stream&);
      //
      template<typename Stream>
      static constexpr void skip(Stream&);
   };
}

#include "./property_declaration.inl"