#pragma once
#include <cstdint>

namespace dovah {
   class form_stub;
}

namespace dovah::loaded_forms::components::conditions {
   struct working_event_parameters {
      constexpr bool operator==(const working_event_parameters&) const noexcept = default;
      
      uint16_t   function = 0;
      uint16_t   member   = 0;
      form_stub* form     = nullptr;
   };
}