#pragma once
#include <glm/glm.hpp>
#include "_vulkan.h"

namespace vulkanDK {
   // Suitable for use as a uniform buffer object.
   struct scene_global_state {
      //
      // Vulkan expects precise member aligmnent; see: <https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/chap15.html#interfaces-resources-layout>
      //
      alignas(16) glm::mat4 view;
      alignas(16) glm::mat4 proj;
   };
}