#include "vertex.h"

namespace vulkanDK {
   /*static*/ std::array<VkVertexInputAttributeDescription, 3> vertex::getAttributeDescriptions() {
      return {
         VkVertexInputAttributeDescription{
            .location = 0, // should match the location value in the shader's code
            .binding  = 0,
            .format   = VK_FORMAT_R32G32B32_SFLOAT, // vec3
            .offset   = offsetof(vertex, pos),
         },
         VkVertexInputAttributeDescription{ // vertex color
            .location = 1,
            .binding  = 0,
            .format   = VK_FORMAT_R32G32B32_SFLOAT,
            .offset   = offsetof(vertex, color),
         },
         VkVertexInputAttributeDescription{ // UVs
            .location = 2,
            .binding  = 0,
            .format   = VK_FORMAT_R32G32_SFLOAT,
            .offset   = offsetof(vertex, texCoord),
         },
      };
   }
   /*static*/ VkVertexInputBindingDescription vertex::getBindingDescription() {
      return VkVertexInputBindingDescription{
         .binding   = 0,
         .stride    = sizeof(vertex),
         .inputRate = VK_VERTEX_INPUT_RATE_VERTEX, // used for non-instanced rendering
      };
   }
}