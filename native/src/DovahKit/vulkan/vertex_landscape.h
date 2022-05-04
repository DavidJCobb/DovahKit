#pragma once
#include <array>
#include <glm/glm.hpp>
#include "_vulkan.h"
#include "vertex_metadata.h"

namespace vulkanDK {
   struct vertex_landscape {
      using blend_value_t = float;

      // no position attribute needed; the vertex shader can compute that from the vertex indices
      glm::vec3 color  = { 1.0F, 1.0F, 1.0F };
      glm::vec3 normal = { 0.0F, 0.0F, 1.0F };
      float     height;
      std::array<blend_value_t, 6> blends = { 0, 0, 0, 0, 0, 0 };
      
      static constexpr auto attribute_descriptions() {
         constexpr auto data = vertex_attributes_from_data<
            vertex_attribute_offset<decltype(color),  offsetof(vertex_landscape, color)>,
            vertex_attribute_offset<decltype(normal), offsetof(vertex_landscape, normal)>,
            vertex_attribute_offset<decltype(height), offsetof(vertex_landscape, height)>,
            vertex_attribute_offset<decltype(blends), offsetof(vertex_landscape, blends)>
         >(0);
         return data;
      }
      static constexpr VkVertexInputBindingDescription binding_description() {
         return VkVertexInputBindingDescription{
            .binding   = 0,
            .stride    = sizeof(vertex_landscape),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX, // used for non-instanced rendering
         };
      }
   };
}