#pragma once
#include <cstdint>
#include "helpers/arrays/make.h"

namespace vulkanDK::config {
   inline constexpr bool enable_validation_layers = false
      #if _DEBUG
         || true
      #endif
   ;

   inline constexpr const auto desired_validation_layers = cobb::arrays::make(
      "VK_LAYER_KHRONOS_validation"
   );
}