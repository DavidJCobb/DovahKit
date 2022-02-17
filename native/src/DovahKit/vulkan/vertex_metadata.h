#pragma once
#include <array>
#include <type_traits>
#include <glm/glm.hpp>
#include "_vulkan.h"

namespace vulkanDK {
   template<typename T, size_t Offset> struct vertex_attribute_offset {
      public:
         vertex_attribute_offset() = delete;
         ~vertex_attribute_offset() = delete;

         using type = T;
         static constexpr size_t   offset = Offset;
         static constexpr VkFormat format = ([]() constexpr {
            if constexpr (std::is_same_v<type, glm::vec1>)
               return VK_FORMAT_R32_SFLOAT;
            if constexpr (std::is_same_v<type, glm::vec2>)
               return VK_FORMAT_R32G32_SFLOAT;
            if constexpr (std::is_same_v<type, glm::vec3>)
               return VK_FORMAT_R32G32B32_SFLOAT;
            if constexpr (std::is_same_v<type, glm::vec4>)
               return VK_FORMAT_R32G32B32A32_SFLOAT;
            if constexpr (std::is_same_v<type, float>)
               return VK_FORMAT_R32_SFLOAT;
            return VK_FORMAT_UNDEFINED;
         })();
   };
   template<typename... Attributes> inline consteval std::array<VkVertexInputAttributeDescription, sizeof...(Attributes)> vertex_attributes_from_data(uint32_t binding) {
      constexpr auto size    = sizeof...(Attributes);
      constexpr auto offsets = std::array<size_t,   size>{ Attributes::offset... };
      constexpr auto formats = std::array<VkFormat, size>{ Attributes::format... };
      //
      std::array<VkVertexInputAttributeDescription, size> out = {};
      for (size_t i = 0; i < size; ++i) {
         out[i].binding  = binding;
         out[i].location = i;
         out[i].offset   = offsets[i];
         out[i].format   = formats[i];
      }
      return out;
   };
}