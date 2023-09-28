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
      //
      // We need to convert our rotation from Skyrim axis conventions to the 
      // Vulkan clip-space axis conventions. Then, we need to construct a view 
      // matrix.
      // 
      //    SKYRIM AXIS CONVENTIONS
      //    Intrinsic lefthanded (clockwise) XYZ
      //    +X = Right
      //    +Y = Forward
      //    +Z = Up
      // 
      //    VULKAN CLIP-SPACE AXIS CONVENTIONS
      //    Righthanded (counterclockwise)
      //    +X = Right
      //    +Y = Down
      //    +Z = Forward (Depth)
      // 
      // We perform a clip-space Y-axis flip within our projection matrix, which 
      // has the effect of swapping from lefthanded to righthanded. As such, the 
      // view matrix that we create here must produce lefthanded output. However, 
      // the view matrix will still be responsible for swapping which axes affect 
      // which physical dimensions, i.e. which way is vertical.
      // 
      // Through trial and error, I've determined that we can get our axes aimed 
      // in the right directions through these transformations:
      // 
      //  - Subtract 90deg from X, and then negate it.
      //  - Pass Y verbatim.
      //  - Negate Z.
      // 
      // Don't even goddamn ask me how that works. I've spent the last eight or 
      // so hours at *least* trying to figure this out and I'm not any closer to 
      // understanding it.
      // 
      // As for the basics of view matrices, though, this lovely SO answer is a 
      // great explanation:
      // 
      //    https://stackoverflow.com/a/22621286
      //
      // Let T be the transformation matrix.
      // Let R be the rotation matrix.
      // Let V be the view matrix.
      // 
      // Let Ex, Ey, and Ez be the Euler rotations.
      // Let Px, Py, and Pz be the position coordinates.
      // 
      // Let X, Y, and Z be the rotation matrices for each Euler component.
      // Let M(...) be the rotation matrix for a given Euler component.
      // 
      //    V = (TR)^-1
      // 
      //    V = R^-1 * T^-1
      //      = R^-1 * TranslationMatrix(-Px, -Py, -Pz)
      // 
      //    R    = XYZ
      //    R^-1 = (XYZ)^-1
      //    R^-1 = Z^-1 * Y^-1 * X^-1
      // 
      // The inverse of a single-axis rotation matrix is its transpose. By 
      // happenstance, transposing the matrix is equivalent to flipping the 
      // signs on all sine calls, which in turn is equivalent to negating 
      // the angle (because sin(-a) = -sin(a), but cos(-a) = cos(a)). Ergo:
      // 
      //    R^-1 = M(-Ez) * M(-Ey) * M(-Ex)
      // 
      // The inverse of a ZYX rotation matrix, then, can be said to be a 
      // "-X-Y-Z" matrix; and the inverse of an XYZ rotation matrix can be 
      // said to be an "-Z-Y-X" matrix.
      //

      // -----------------------------------------------------------------

      // (Intrinsic XYZ)^-1 = Intrinsic -Z-Y-X
      //
      // In addition, we need to convert from the Skyrim axes to the view-
      // space axes.

      glm::mat4 rot = cobb::glm::euler_intrinsic_xyz_to_mat<cobb::glm::handedness::left>(glm::vec3{
         -(this->_rotation.x - ninety_degrees),
         this->_rotation.y,
         -this->_rotation.z
      });
      this->_view_matrix = glm::translate(rot, -this->_position);
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
         #if 0
         std::swap(move.z, move.y);
         move.z = -move.z;
         //
         // Now, we need to make it world-relative.
         //
         move = glm::inverse(glm::mat3x3(this->_view_matrix)) * move;
         this->_position += move;
         #else
         this->_position += this->camera_rotation_matrix() * move;
         #endif
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

   glm::mat4 camera::camera_matrix() const {
      return vulkanDK::glm_transform_from_beth({ 0, 0, 0 }, this->_rotation, 1.0);
   }
   glm::mat3 camera::camera_rotation_matrix() const {
      return glm::mat3(cobb::glm::euler_intrinsic_xyz_to_mat<cobb::glm::handedness::left>(this->_rotation));
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