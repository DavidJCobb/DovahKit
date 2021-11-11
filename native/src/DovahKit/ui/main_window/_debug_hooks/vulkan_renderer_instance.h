#pragma once
#include "_base.h"

namespace DovahKitDebug::features {
   struct vulkan_renderer_instance : debug_feature {
      static constexpr const char* name = "Test Vulkan refactor";
      static void execute(QWidget* from);
   };
}