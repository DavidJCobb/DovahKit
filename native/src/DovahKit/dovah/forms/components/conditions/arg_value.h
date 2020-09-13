#pragma once
#include "../../_common.h"

namespace dovah::loaded_forms::components {
   struct condition_arg_value {
      union {
         uint32_t  dword = 0;
         float     float32;
         form_id_t formID;
      };
      std::string string;

      condition_arg_value() : dword(0) {}
   };
}
