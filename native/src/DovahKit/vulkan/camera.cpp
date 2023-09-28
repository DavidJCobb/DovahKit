#include "./camera.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/rotate_vector.hpp> // glm::rotate on vectors

namespace {
   constexpr float epsilon    = 0.00001;
   constexpr float epsilon_sq = epsilon * epsilon;
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

   //
   // Camera axes (matching Skyrim):
   // 
   //    Lefthanded (clockwise) extrinsic XYZ Euler
   //    +X = Right
   //    +Y = Forward
   //    +Z = Up
   // 
   // View-space axes:
   // 
   //    Lefthanded (clockwise) Euler
   //    +X = Right
   //    +Y = Down
   //    +Z = Forward (Depth)
   //

   void camera::_fire_callback(bool translated, bool rotated) {
      if (!this->_callback.functor)
         return;
      (this->_callback.functor)(this->_callback.context, translated, rotated);
   }
   void camera::_update_view_matrix() {
      auto rot = glm::eulerAngleZYX(-this->_rotation.z, -this->_rotation.y, -this->_rotation.x);
      this->_view_matrix = glm::translate(glm::inverse(rot), -this->_position);
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
         // We need to start with a vector that's relative to the camera's reference frame. 
         // However, the camera uses different axes than we expect.
         // 
         // We want to be able to treat the camera as just another object, and objects in 
         // Skyrim use these directions:
         // 
         //  +X = Right
         //  +Y = Forward
         //  +Z = Up
         // 
         // However, the camera's local axes are:
         // 
         //  +X = Right
         //  +Y = Down
         //  +Z = Forward (Depth)
         // 
         // So to start with, we need to swap and negate some axes.
         //
         std::swap(move.z, move.y);
         move.z = -move.z;
         //
         // Now, we need to make it world-relative.
         //
         move = glm::inverse(glm::mat3x3(this->_view_matrix)) * move;
         this->_position += move;
      }

      if (do_move || do_turn) {
         this->_update_view_matrix();
         this->_fire_callback(do_move, do_turn);
         return true;
      }
      return false;
   }

   bool camera::arcball(glm::vec3 pivot, glm::vec3 turn) {
      //this->aim_at_target(pivot); // BROKEN

      if constexpr (false) { // force look at target (BROKEN)
         //
         // If we're looking at a steep vertical angle, let's try to maintain it.
         //
         glm::vec3 up = glm::normalize(-this->_view_matrix[0]);
         if (fabs(up.z) < 0.8) {
            up = glm::vec3{ 0.0, 0.0, 1.0 };
         } else {
            glm::vec3 forward = glm::normalize(-this->_view_matrix[1]);
            glm::vec3 test    = glm::cross(forward, up);
            if (glm::length2(test) < 0.1) {
               //
               // The `pivot` object is in the same direction as our up-vector (or in the 
               // exact opposite direction), so we can't keep the up-vector.
               //
               up = glm::vec3{ 0.0, 0.0, 1.0 };
            }
         }
         //
         auto look = glm::lookAtLH(this->_position, pivot, up);
         glm::extractEulerAngleXYZ(look, this->_rotation.x, this->_rotation.y, this->_rotation.z);
         this->_rotation *= -1;
      }

      bool do_turn = glm::length2(turn) > epsilon_sq;

      if (do_turn) {
         this->_rotation -= turn;

         auto distance = glm::distance(this->_position, pivot);
         //
         glm::vec4 offset = { 0, 0, 1, 0 };
         offset = glm::eulerAngleZYX(-this->_rotation.z, -this->_rotation.y, -this->_rotation.x) * offset;
         offset *= distance;
         //
         this->_position = glm::vec3(offset) + pivot;

         this->_update_view_matrix();
      }

      this->_fire_callback(do_turn, true);

      return true;
   }

   // TODO: BROKEN
   void camera::aim_at_target(glm::vec3 pivot) {
      glm::vec3 forward = glm::normalize(pivot - this->_position);
      glm::vec3 up      = glm::normalize(this->_view_matrix[2]);
      if (fabs(up.z) < 0.8) {
         //
         // If we're not looking at a steep vertical angle already, then discard the 
         // camera's up-vector and recompute it from the forward vector.
         //
         up = glm::vec3{ 0.0, 0.0, 1.0 };
      } else {
         //
         // Let's test to see if we can maintain camera-up.
         //
         auto test = glm::cross(forward, up);
         if (glm::length2(test) < (1.0 - epsilon)) {
            //
            // The `pivot` object is in the same direction as our up-vector (or in the 
            // exact opposite direction), so we can't keep the up-vector.
            //
            up = glm::vec3{ 0.0, 0.0, 1.0 };
         }
      }
      glm::vec3 side;
      //
      if constexpr (righthanded) {
         side = glm::normalize(glm::cross(forward, up));
         up   = glm::cross(side, forward);
      } else {
         side = glm::normalize(glm::cross(up, forward));
         up   = glm::cross(forward, side);
      }

      glm::mat4 matrix = glm::mat4(1);
      matrix[0][0] = side.x;
      matrix[1][0] = side.y;
      matrix[2][0] = side.z;
      matrix[0][1] = up.x;
      matrix[1][1] = up.y;
      matrix[2][1] = up.z;
      if constexpr (righthanded) {
         matrix[0][2] = -forward.x;
         matrix[1][2] = -forward.y;
         matrix[2][2] = -forward.z;
      } else {
         matrix[0][2] = forward.x;
         matrix[1][2] = forward.y;
         matrix[2][2] = forward.z;
      }
      matrix[3] = glm::vec4(this->_position, 1.0F);

      this->_view_matrix = matrix;
      glm::extractEulerAngleXYZ(matrix, this->_rotation.x, this->_rotation.y, this->_rotation.z);
      this->_rotation.y = 0; // no roll

      this->_fire_callback(false, true);
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