#pragma once
#include <glm/glm.hpp>

struct DKVulkanCameraUpdate {
   double delta_seconds = 0.0; // delta time; 0 = no change
   struct {
      float     speed = 0.0; // per second
      glm::vec3 direction = { 0, 0, 0 }; // world-relative direction
      bool      scale_by_delta = true;
   } move;
   struct {
      float speed = glm::radians(1.0F); // radians per second
      float yaw   = 0.0; // clamped to [-1, 1]
      float pitch = 0.0; // clamped to [-1, 1]
      float roll  = 0.0; // clamped to [-1, 1]
      bool  scale_by_delta = true;
   } turn;

   inline bool has_turn() const noexcept { return this->turn.speed != 0 && (this->turn.yaw || this->turn.pitch || this->turn.roll); }
};