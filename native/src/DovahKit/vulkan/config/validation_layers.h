#pragma once
#include <cstdint>

namespace vulkanDK::config {
   inline constexpr bool enable_validation_layers = false
      #if _DEBUG
         || true
      #endif
   ;

   const std::vector<const char*> desired_validation_layers = {
      "VK_LAYER_KHRONOS_validation"
   };
}