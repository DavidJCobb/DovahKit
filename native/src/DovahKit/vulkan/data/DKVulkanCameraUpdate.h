#pragma once
#include <glm/glm.hpp>

struct DKVulkanCameraUpdate {
   float delta_seconds = 0.0; // delta time; 0 = no scale (e.g. for one-time "jump")
   struct {
      float     speed = 0.0; // per second
      glm::vec3 direction = { 0, 0, 0 }; // world-relative direction
   } move;
   struct {
      float     speed    = glm::radians(1.0F); // radians per second
      glm::vec3 rotation = { 0, 0, 0 }; // Euler radians
   } turn;
};