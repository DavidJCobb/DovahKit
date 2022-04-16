#pragma once
#include <array>
#include <cstdint>
#include <string>
#include "_vulkan.h"

namespace vulkanDK {
   struct spir_v_entry_point {
      union xyz {
         xyz() : list({}) {}

         std::array<uint32_t, 3> list;
         struct {
            uint32_t x;
            uint32_t y;
            uint32_t z;
         };
      };

      struct execution_mode_data {
         xyz LocalSize;
      };
      
      std::string name;
      uint32_t    id = 0;
      VkShaderStageFlags stages = 0;
      //
      execution_mode_data execution_modes;
   };
}