#pragma once
#include <array>
#include <glm/glm.hpp>
#include "_vulkan.h"
#include "vertex_metadata.h"

namespace vulkanDK {
   struct vertex {
      glm::vec3 pos;
      glm::vec3 color;
      glm::vec2 uv;
      glm::vec3 normal;
      glm::vec3 tangent;
      glm::vec3 bitangent;
      //
      static constexpr std::array<VkVertexInputAttributeDescription, 6> getAttributeDescriptions() {
         constexpr auto data = vertex_attributes_from_data<
            vertex_attribute_offset<decltype(pos),       offsetof(vertex, pos)>,
            vertex_attribute_offset<decltype(color),     offsetof(vertex, color)>,
            vertex_attribute_offset<decltype(uv),        offsetof(vertex, uv)>,
            vertex_attribute_offset<decltype(normal),    offsetof(vertex, normal)>,
            vertex_attribute_offset<decltype(tangent),   offsetof(vertex, tangent)>,
            vertex_attribute_offset<decltype(bitangent), offsetof(vertex, bitangent)>
         >(0);
         return data;
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