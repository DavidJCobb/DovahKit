#pragma once
#include <glm/glm.hpp>
#include "_vulkan.h"

namespace vulkanDK {
   // Suitable for use as a uniform buffer object.
   struct scene_global_state {
      //
      // Vulkan expects precise member aligmnent; see: <https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/chap15.html#interfaces-resources-layout>
      // 
      // The "view" matrix is the INVERSE of the camera's matrix. That is: the view matrix is the 
      // inverse of the transformation matrix one would use to place an object at the same world-
      // relative position as the camera, facing the same direction as the camera. This means that 
      // in order to modify the camera's own transform, you must produce a transform, invert it, 
      // and *then* write it into (view).
      //
      alignas(16) glm::mat4 view;
      alignas(16) glm::mat4 proj;
   };
}