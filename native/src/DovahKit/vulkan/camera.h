#pragma once
#include <glm/glm.hpp>

namespace vulkanDK {
   class camera {
      public:
         static constexpr const bool righthanded = true;

         using coordinate_change_callback = void(*)(void* context, bool translated, bool rotated);

      protected:
         glm::vec3 _position;
         glm::vec3 _rotation; // pitch, roll, yaw
         glm::mat4 _view_matrix;

         struct {
            void* context = nullptr;
            coordinate_change_callback functor = nullptr;
         } _callback;

         void _fire_callback(bool translated, bool rotated);
         void _update_view_matrix();

      public:
         bool translate_absolute(glm::vec3 move);
         bool translate_relative(glm::vec3 move);

         // Apply camera-relative movement and turning. Returns true if any change is made.
         bool adjust(glm::vec3 move, glm::vec3 turn);

         // Orbit around a pivot position. Turn angles are camera-relative. If the camera is 
         // not already aiming at the pivot, it will be forced to aim before the arc movement 
         // is applied.
         bool arcball(glm::vec3 pivot, glm::vec3 turn);

      public:
         constexpr const glm::vec3& position() const { return this->_position; }
         constexpr const glm::vec3& rotation() const { return this->_rotation; }
         constexpr const glm::mat4& view_matrix() const { return this->_view_matrix; }

         glm::mat4 camera_matrix() const;
         glm::mat3 camera_rotation_matrix() const;

         void set_position(const glm::vec3&);
         void set_rotation(const glm::vec3& pitch_roll_yaw_radians);
         void set_coordinates(const glm::vec3& pos, const glm::vec3& pitch_roll_yaw_radians);
   };
}