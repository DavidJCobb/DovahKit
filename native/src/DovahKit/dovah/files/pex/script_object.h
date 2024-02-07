#pragma once
#include <string>
#include <vector>
#include "./global_variable_declaration.h"
#include "./property_declaration.h"
#include "./state.h"
#include "./tabled_string.h"

namespace dovah::pex {
   struct script_object {
      tabled_string name;
      tabled_string superclass;
      tabled_string docstring;
      uint32_t      user_flags = 0;
      tabled_string auto_state_name;

      std::vector<global_variable_declaration> variables;
      std::vector<property_declaration> properties;
      std::vector<state> states;

      template<typename Stream>
      constexpr void read(Stream&);
      //
      template<typename Stream>
      static constexpr void skip(Stream&);
   };
}

#include "./script_object.inl"