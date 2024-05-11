#pragma once
#include <cstdint>
#include "../../../core.h"

namespace dovah::loaded_forms::components::conditions {
   struct event_parameters {
      uint16_t         function = 0;
      uint16_t         member   = 0;
      form_reference_t form;
   };
}