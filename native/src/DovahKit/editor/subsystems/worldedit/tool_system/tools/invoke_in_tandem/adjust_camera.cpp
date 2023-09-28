#include "./adjust_camera.h"
#include "helpers/math/rotation/unit_conversion.h"

#include "editor/ini/main.h"
#include "vulkan/data/camera_coordinate_change.h"

#include "../../../enums/camera_speed_flags.h"
#include "../../../core.h"

namespace {
   namespace worldedit_ini_settings {
      using namespace dovahkit::ini::main::worldedit;
   }

   // If enabled, "held" movement vectors (see comments below) will be normalized if their length 
   // is greater than one. This prevents "strafe-running" and other speed quirks. This shouldn't 
   // be necessary for gamepad input, but would be needed for keyboard input.
   constexpr const bool normalize_super_movements = true;
}

namespace dovahkit::subsystems::worldedit::tools::tandem {
   /*static*/ void adjust_camera::_invoke_impl(
      const move_camera::response*  params_move,
      const orbit_camera::response* params_orbit,
      const turn_camera::response*  params_turn
   ) {
      auto& worldedit_core = core::get();

      if (params_move || params_turn) {
         vulkanDK::data::camera_coordinate_change update;
         if (params_move) {
            //
            // The "move" params include two movement vectors: the "instant" vector and the "held" 
            // vector. The former vector represents sudden jumps triggered by instantaneous inputs 
            // e.g. the immediate press or release of a button. The latter vector represents speeds 
            // per second for movements triggered by sustained inputs, like holding a button down.
            // 
            // The "held" vector should be scaled by any relevant movement speed prefs, whereas the 
            // "instant" vector should not. Neither vector should be scaled by the frame delta: the 
            // "held" vector will already have been scaled within Worldinput, so multiplying in any 
            // speed-per-second values is all that's needed.
            //
            const auto& data = *params_move;
            update.move = data.held.to_struct<glm::vec3>();
            if constexpr (normalize_super_movements) {
               const auto len = glm::length(update.move);
               if (len > 1.0)
                  update.move /= len;
            }
            //
            // Apply movement speeds per second:
            //
            update.move *= worldedit_core.get_camera_move_speed();
            update.move += data.instant.to_struct<glm::vec3>();
         }
         if (params_turn) {
            //
            // Camera turning makes the same "held" and "instant" distinction as camera movement.
            //
            const auto& data = *params_turn;
            update.turn = data.held.to_struct<glm::vec3>();
            update.turn.z *= cobb::degrees_to_radians_mult * worldedit_ini_settings::fTurnSpeedDegreesPerSecondX.get_current_value<double>();
            update.turn.x *= cobb::degrees_to_radians_mult * worldedit_ini_settings::fTurnSpeedDegreesPerSecondY.get_current_value<double>();

            update.turn += data.instant.to_struct<glm::vec3>();
         }
         worldedit_core.adjust_camera(update);
      }

      if (params_orbit) {
         glm::vec3 turn = params_orbit->held.to_struct<glm::vec3>();
         turn.z *= cobb::degrees_to_radians_mult * worldedit_ini_settings::fTurnSpeedDegreesPerSecondX.get_current_value<double>();
         turn.x *= cobb::degrees_to_radians_mult * worldedit_ini_settings::fTurnSpeedDegreesPerSecondY.get_current_value<double>();
         //
         turn += params_orbit->instant.to_struct<glm::vec3>();

         worldedit_core.orbit_camera(
            params_orbit->target,
            turn
         );
      }
   }
}