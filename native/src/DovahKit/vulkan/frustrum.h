#pragma once
#include <array>
#include <glm/glm.hpp>
#include "helpers/glm/type_traits.h"

namespace vulkanDK {
   //
   // Information about a view frustrum.
   //
   struct frustrum {
      static constexpr size_t plane_count = 2;

      union plane {
         static constexpr size_t point_count = 5;

         std::array<glm::vec3, point_count> list;
         struct {
            glm::vec3 center;
            glm::vec3 upper_left;
            glm::vec3 upper_right;
            glm::vec3 lower_left;
            glm::vec3 lower_right;
         };
      };
      static_assert(sizeof(plane) == sizeof(glm::vec3) * plane::point_count, "Sanity-check static assertion: union validity: vec3s are contiguous?");

      union {
         alignas(glm::vec3) std::array<glm::vec3, (plane::point_count * plane_count)> points = {};
         alignas(glm::vec3) struct {
            alignas(glm::vec3) plane near;
            alignas(glm::vec3) plane far;
         } planes;
      };
      struct {
         struct {
            float w = 0.0F;
            float h = 0.0F;
         } near;
         struct {
            float w = 0.0F;
            float h = 0.0F;
         } far;
      } bounds;

      static_assert(sizeof(planes) == sizeof(glm::vec3) * (plane::point_count * plane_count), "Sanity-check static assertion: union validity: vec3s are contiguous?");

      frustrum() : points({}) {}

      inline glm::vec3 center() const { return (this->planes.far.center - this->planes.near.center) * 0.5F; }

      frustrum& operator*=(const glm::mat4& x) {
         for (auto& item : this->points)
            item = x * glm::vec4(item, 1.0F);
         return *this;
      }
      frustrum& operator*=(float x) {
         for (auto& item : this->points)
            item *= x;
         return *this;
      }
      template<typename T>
      frustrum operator*(const T& x) const {
         frustrum out = *this;
         out *= x;
         return out;
      }
   };
}