#pragma once
#include "../../_common.h"

namespace dovah::loaded_forms::components {
   struct condition_arg_value {
      union {
         uint32_t dword;
         float    float32;
         form_reference_t form;
      };
      std::string string;

      condition_arg_value() : form(nullptr) {}
   };
}
