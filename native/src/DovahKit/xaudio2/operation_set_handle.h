#pragma once
#include <cstdint>
namespace dovahkit::xaudio2 {
   class core_interface;
}

namespace dovahkit::xaudio2 {
   struct operation_set_handle {
      friend core_interface;
      public:
         const uint32_t id = 0;
   };
}