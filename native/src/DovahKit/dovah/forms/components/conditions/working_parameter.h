#pragma once
#include <cstdint>
#include <string>
#include <variant>

namespace dovah {
   class form_stub;
}

namespace dovah::loaded_forms::components::conditions {
   using working_parameter = std::variant<
      std::monostate,
      uint32_t,   // alias, event, int_unsigned, package_data, quest_stage
      char,       // character
      float,      // float32
      int32_t,    // enumeration, int_signed
      form_stub*, // form
      std::string // string
   >;
}