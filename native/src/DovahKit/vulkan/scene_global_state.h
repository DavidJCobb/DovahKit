#pragma once
#include <glm/glm.hpp>
#include "_vulkan.h"

namespace vulkanDK {
   // Suitable for use as a uniform buffer object.
   struct scene_global_state {
      //
      // Vulkan expects precise member aligmnent; see: <https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/chap15.html#interfaces-resources-layout>
      // 
      // The "view" matrix isn't the camera matrix, nor the inverse of the camera matrix. 
      // Rather, it is the inverse of each part of the camera matrix:
      // 
      //    glm::translate(
      //       glm::inverse(rotation),
      //       glm::inverse(glm::translate(glm::mat4(1), position))
      //    );
      // 
      // Inverting the "translation" part of a transformation matrix inverts the translation 
      // itself; ergo this code, which translates by an inverted position, is equivalent:
      // 
      //    glm::translate(glm::inverse(rotation), -position);
      //
      alignas(16) glm::mat4 view;
      alignas(16) glm::mat4 proj;
   };
}