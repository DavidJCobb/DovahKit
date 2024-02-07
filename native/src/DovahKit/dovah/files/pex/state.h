#pragma once
#include <string>
#include <vector>
#include "./compiled_function.h"
#include "./tabled_string.h"

namespace dovah::pex {
   struct state {
      tabled_string name; // empty string for default state
      std::vector<compiled_named_function> functions;

      template<typename Stream>
      constexpr void read(Stream&);
      //
      template<typename Stream>
      static constexpr void skip(Stream&);
   };
}

#include "./state.inl"