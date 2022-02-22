#pragma once
#include "../_vulkan.h"

namespace vulkanDK {
   template<uint32_t constantID, typename T, size_t Offset> constexpr VkSpecializationMapEntry specialization_map_entry_for_member() {
      return VkSpecializationMapEntry{
         .constantID = constantID,
         .offset     = Offset,
         .size       = sizeof(T),
      };
   }
}