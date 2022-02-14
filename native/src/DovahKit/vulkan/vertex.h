#pragma once
#include <array>
#include <glm/glm.hpp>
#include "_vulkan.h"

namespace vulkanDK {
   struct vertex {
      glm::vec3 pos;
      glm::vec3 color;
      glm::vec2 uv;
      glm::vec3 normal;
      //
      static constexpr std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
         return std::array{
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
               .offset   = offsetof(vertex, uv),
            },
            VkVertexInputAttributeDescription{ // normals
               .location = 3,
               .binding  = 0,
               .format   = VK_FORMAT_R32G32B32_SFLOAT, // vec3
               .offset   = offsetof(vertex, normal),
            },
         };
      }
      static constexpr VkVertexInputBindingDescription getBindingDescription() {
         return VkVertexInputBindingDescription{
            .binding   = 0,
            .stride    = sizeof(vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX, // used for non-instanced rendering
         };
      }
   };
}