#pragma once
#include <array>
#include <type_traits>
#include <glm/glm.hpp>
#include "helpers/type_traits/is_std_array.h"
#include "_vulkan.h"

namespace vulkanDK {
   template<typename T, size_t Offset> struct vertex_attribute_offset {
      public:
         vertex_attribute_offset() = delete;
         ~vertex_attribute_offset() = delete;

         using type = T;
         static constexpr size_t   offset = Offset;
         static constexpr size_t   count  = 1;
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

   template<typename T, size_t Offset> requires cobb::is_std_array<T> struct vertex_attribute_offset<T, Offset> {
      public:
         vertex_attribute_offset() = delete;
         ~vertex_attribute_offset() = delete;

         using type = typename T::value_type;
         static constexpr size_t   offset = Offset;
         static constexpr size_t   count  = std::tuple_size_v<T>;
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

   template<typename... Attributes> inline consteval std::array<VkVertexInputAttributeDescription, (Attributes::count + ...)> vertex_attributes_from_data(uint32_t binding) {
      constexpr auto size    = sizeof...(Attributes);
      constexpr auto offsets = std::array<size_t,   size>{ Attributes::offset... };
      constexpr auto formats = std::array<VkFormat, size>{ Attributes::format... };
      constexpr auto sizeofs = std::array<size_t,   size>{ sizeof(Attributes::type)...  };
      constexpr auto counts  = std::array<size_t,   size>{ Attributes::count...  };
      //
      std::array<VkVertexInputAttributeDescription, (Attributes::count + ...)> out = {};
      size_t j = 0;
      for (size_t i = 0; i < size; ++i) {
         for (size_t k = 0; k < counts[i]; ++j, ++k) {
            out[j].binding  = binding;
            out[j].location = j;
            out[j].offset   = offsets[i] + (sizeofs[i] * k);
            out[j].format   = formats[i];
         }
      }
      return out;
   };
}