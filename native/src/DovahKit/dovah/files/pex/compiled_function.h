#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "./compiled_opcode.h"
#include "./tabled_string.h"
#include "./variable_declaration.h"

namespace dovah::pex {
   struct compiled_function {
      struct flag {
         flag() = delete;
         enum {
            global = 0x01,
            native = 0x02,
         };
      };
      
      tabled_string return_type;
      tabled_string docstring;
      uint32_t      flags          = 0; // user_flags
      uint8_t       function_flags = 0; // function::flag
      //
      std::vector<variable_declaration> arguments;
      std::vector<variable_declaration> locals;
      std::vector<compiled_opcode>      opcodes;

      template<typename Stream>
      constexpr void read(Stream&);
      //
      template<typename Stream>
      static constexpr void skip(Stream&);
   };
   
   struct compiled_named_function : compiled_function {
      tabled_string name;

      template<typename Stream>
      constexpr void read(Stream&);
      //
      template<typename Stream>
      static constexpr void skip(Stream&);
   };
}

#include "./compiled_function.inl"