#pragma once
#include <cstdint>

namespace dovah {
   enum class magic_delivery_type : uint32_t {
      self,
      touch,
      aimed,
      target_actor,
      target_location,
   };
}