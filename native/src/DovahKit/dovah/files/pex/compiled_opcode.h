#pragma once
#include <vector>
#include "./opcode_type.h"
#include "./value.h"

namespace dovah::pex {
   struct compiled_opcode {
      opcode_type type = opcode_type::nop;
      std::vector<value> operands;

      template<typename Stream>
      constexpr void read(Stream&);
      //
      template<typename Stream>
      static constexpr void skip(Stream&);
   };
}

#include "./compiled_opcode.inl"