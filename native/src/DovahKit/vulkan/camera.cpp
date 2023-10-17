#include "./camera.h"
#include "helpers/glm/euler_to_matrix.h"
#include "helpers/math/rotation/unit_conversion.h"
#include "./helpers/glm_transform_from_beth.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/rotate_vector.hpp> // glm::rotate on vectors

namespace {
   constexpr const float epsilon    = 0.00001;
   constexpr const float epsilon_sq = epsilon * epsilon;
   //
   constexpr const float ninety_degrees = 90.0F * cobb::degrees_to_radians_mult;
}

//
// Things to look into:
// 
//  - Feature to constrain the pitch to min/max values, like in Reach. Possible 
//    answer here?
//    https://stackoverflow.com/questions/31612839/how-to-avoid-a-sphere-from-turning-upside-down-when-user-rotates-it-in-opengl-e/31786698
// 
//  - Use a quaternion to store rotations internally? Here's how to do it for a 
//    first-person camera:
//    https://gamedev.stackexchange.com/a/162486
//

namespace vulkanDK {
   void camera::_fire_callback(bool translated, bool rotated) {
      if (!this->_callback.functor)
         return;
      (this->_callback.functor)(this->_callback.context, translated, rotated);
   }
   void camera::_update_view_matrix() {
      //
      // In clip-space, +X is to the right, +Y is down, and +Z is forward (depth). We 
      // want an axis convention where +X is to the right, +Y forward, and +Z is up. 
      // Both of these conventions are lefthanded, so negating or swapping axes isn't 
      // an option... but we can just rotate back 90 degrees on the clip-space X-axis 
      // to bring the clip axes in line with the Skyrim axes.
      // 
      // We ONLY need the -90deg offset when going from world space to view/clip space. 
      // The offset is fed into the view matrix but IS NOT part of the camera matrix!
      // 
      // As for our rotation order?
      //
      // We want to apply roll, then yaw, then pitch... but remember: the view matrix is 
      // the inverse of the camera matrix. We need to apply our angles in the opposite 
      // order *and* negate them.
      // 
      auto rot = cobb::glm::euler_intrinsic_xzy_to_mat<cobb::glm::handedness::left>(glm::vec3{
         -(this->_rotation.x - ninety_degrees),
         -this->_rotation.y,
         -this->_rotation.z
      });
      rot = glm::translate(rot, -this->_position);
      this->_view_matrix = rot;
      return;
   }

   bool camera::translate_absolute(glm::vec3 move) {
      if (glm::length2(move) < epsilon_sq)
         return false;

      this->_position += move;
      this->_update_view_matrix();
      this->_fire_callback(true, false);
      return true;
   }
   bool camera::translate_relative(glm::vec3 move) {
      if (glm::length2(move) < epsilon_sq)
         return false;

      this->_position += this->camera_rotation_matrix() * move;
      this->_update_view_matrix();
      this->_fire_callback(true, false);
      return true;
   }

   bool camera::adjust(glm::vec3 move, glm::vec3 turn) {
      bool do_move = (glm::length2(move) >= epsilon_sq);
      bool do_turn = (glm::length2(turn) >= epsilon_sq);

      if (do_turn) {
         //
         // Continually modifying  a matrix opens us up to floating-point  inaccuracy and 
         // therefore to "creeping roll" within the camera.  Storing bare Euler angles is 
         // a decent enough way to  prevent this, though it means we have to regenerate a 
         // matrix after each camera adjustment.
         //
         this->_rotation += turn;
      }
      if (do_move) {
         //
         // NOTE: REMEMBER TO UPDATE `translate_relative` TO MATCH THIS!
         // 
         this->_position += this->camera_rotation_matrix() * move;
      }

      if (do_move || do_turn) {
         this->_update_view_matrix();
         this->_fire_callback(do_move, do_turn);
         return true;
      }
      return false;
   }

   bool camera::arcball(glm::vec3 pivot, glm::vec3 turn) {
      bool do_turn = glm::length2(turn) > epsilon_sq;

      if (do_turn) {
         this->_rotation -= turn;

         auto distance = glm::distance(this->_position, pivot);
         glm::vec4 offset = (this->camera_matrix() * glm::vec4{ 0, -1, 0, 0 }) * distance;
         //
         this->_position = glm::vec3(offset) + pivot;

         this->_update_view_matrix();
      }

      this->_fire_callback(do_turn, true);

      return true;
   }

   glm::mat4 camera::camera_matrix() const {
      auto rot = cobb::glm::euler_intrinsic_yzx_to_mat<cobb::glm::handedness::left>(glm::vec3{
         this->_rotation.x,
         this->_rotation.y,
         this->_rotation.z
      });
      //
      // <-- If we wanted to support camera scale, we'd multiply the matrix by the scale here.
      //
      rot[3] = glm::vec4(this->_position, 1.0F);
      return rot;
   }
   glm::mat3 camera::camera_rotation_matrix() const {
      auto rot = cobb::glm::euler_intrinsic_yzx_to_mat<cobb::glm::handedness::left>(glm::vec3{
         this->_rotation.x,
         this->_rotation.y,
         this->_rotation.z
      });
      return glm::mat3(rot);
   }
   glm::mat4 camera::inverse_view_matrix() const {
      auto rot = cobb::glm::euler_intrinsic_yzx_to_mat<cobb::glm::handedness::left>(glm::vec3{
         this->_rotation.x - ninety_degrees,
         this->_rotation.y,
         this->_rotation.z
      });
      rot[3] = glm::vec4(this->_position, 1.0F);
      return rot;
   }

   void camera::set_position(const glm::vec3& v) {
      this->_position = v;
      this->_update_view_matrix();
      this->_fire_callback(true, false);
   }
   void camera::set_rotation(const glm::vec3& radians) {
      this->_rotation = radians;
      this->_update_view_matrix();
      this->_fire_callback(false, true);
   }
   void camera::set_coordinates(const glm::vec3& pos, const glm::vec3& rot) {
      this->_position = pos;
      this->_rotation = rot;
      this->_update_view_matrix();
      this->_fire_callback(true, true);
   }
}