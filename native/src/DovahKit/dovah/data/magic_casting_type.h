#pragma once
#include <cstdint>

namespace dovah {
   enum class magic_casting_type : uint32_t {
      constant_effect,
      fire_and_forget,
      concentration,
      scroll,
   };
}