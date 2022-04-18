#pragma once
#include <array>
#include <cstdint>
#include <glm/glm.hpp>
#include "helpers/unreachable.h"
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
      alignas(16) glm::vec3 camera_pos;
      alignas(16) glm::vec3 ambient_light_color = { 0, 0, 0 };
      alignas(16) glm::vec3 sun_dir   = glm::normalize(glm::vec3{ 0.1, 0, -1 }); // vector from sun to world
      alignas(16) glm::vec3 sun_color = { 1, 1, 1 };
      alignas(16) glm::mat4 sun_space = glm::mat4(1);
      //
      alignas( 4) std::array<int32_t, 4> shadow_caster_index = { -1, -1, -1, -1 };
      //
      alignas(16) glm::vec3 fog_color_near = { 0, 0, 0 };
      alignas( 4) float     fog_plane_near = 0;
      alignas(16) glm::vec3 fog_color_far  = { 0, 0, 0 };
      alignas( 4) float     fog_plane_far  = 7000;
      alignas( 4) float     fog_power  = 1.0F;
      alignas( 4) float     fog_max    = 1.0F; // max fog
      alignas( 4) float     interior_clip_distance = 0.0F; // maximum draw distance for interior cells only; unused if zero or negative
   };
}