#pragma once
#include <cstdint>
#include <string>

namespace dovah::loaded_forms::components::papyrus {
   struct basic_fragment {
      uint8_t     unknown;
      std::string script;
      std::string function;
   };
}