#pragma once
#include <array>
#include <glm/glm.hpp>
#include "_vulkan.h"

namespace vulkanDK {
   struct vertex {
      glm::vec3 pos;
      glm::vec3 color;
      glm::vec2 texCoord;
      //
      static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
      static VkVertexInputBindingDescription getBindingDescription();
   };
}