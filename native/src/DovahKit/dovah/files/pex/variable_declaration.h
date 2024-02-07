#pragma once
#include <string>
#include "./tabled_string.h"

namespace dovah::pex {
   struct variable_declaration { // "Variable Type" on UESP
      tabled_string name;
      tabled_string type;

      template<typename Stream>
      constexpr void read(Stream&);
      //
      template<typename Stream>
      static constexpr void skip(Stream&);
   };
}

#include "./variable_declaration.inl"
