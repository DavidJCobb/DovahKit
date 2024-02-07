#pragma once
#include <cstdint>
#include <string>
#include "./tabled_string.h"
#include "./value.h"

namespace dovah::pex {
   struct global_variable_declaration { // "Variable" on UESP
      tabled_string name;
      tabled_string type;
      uint32_t      flags = 0; // user_flags
      value         initial_value;

      template<typename Stream>
      constexpr void read(Stream&);
      //
      template<typename Stream>
      static constexpr void skip(Stream&);
   };
}

#include "./global_variable_declaration.inl"