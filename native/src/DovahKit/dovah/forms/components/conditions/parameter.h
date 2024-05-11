#pragma once
#include <cstdint>
#include <string>
#include "../../../data/conditions/parameter_underlying_type.h"
#include "../../../core.h"

namespace dovah::loaded_forms::components::conditions {
   struct parameter {
      union {
         uint32_t dword = 0;
         float    float32;
         int32_t  integer;
      };
      form_reference_t form;
      std::string      string;
      
      dovah::conditions::parameter_underlying_type underlying = dovah::conditions::parameter_underlying_type::none;
   };
}