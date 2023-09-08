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
}

namespace dovahkit::subsystems::worldedit::tools::tandem {
   /*static*/ void adjust_camera::_invoke_impl(const move_camera::response* params_move, const turn_camera::response* params_turn) {
      auto& worldedit_core = core::get();

      vulkanDK::data::camera_coordinate_change update;
      if (params_move) {
         const auto& data = *params_move;
         update.move = { data.x, data.y, data.z };
               
         update.move *= worldedit_ini_settings::fCameraSpeedNormal.get_current_value<double>();
         //
         if (worldedit_core.get_camera_speed_flag(camera_speed_flag::boost))
            update.move *= worldedit_ini_settings::fCameraSpeedMultBoost.get_current_value<double>();
         if (worldedit_core.get_camera_speed_flag(camera_speed_flag::precision))
            update.move *= worldedit_ini_settings::fCameraSpeedMultPrecision.get_current_value<double>();
      }
      if (params_turn) {
         const auto& data = *params_turn;
         update.turn = { data.pitch, data.roll, data.yaw };
         update.turn.z *= cobb::degrees_to_radians_mult * worldedit_ini_settings::fTurnSpeedDegreesPerSecondX.get_current_value<double>();
         update.turn.x *= cobb::degrees_to_radians_mult * worldedit_ini_settings::fTurnSpeedDegreesPerSecondY.get_current_value<double>();
      }

      //
      // NOTE: Movement and look speeds should apply to Hold movements, including those 
      //       triggered by the mouse and joysticks; however, if someone wants to make a 
      //       keybind like "jump 16 units to the left when I press a key," that shouldn't 
      //       have movement or look speeds applied.
      // 
      //       To accomplish this, we'll probably have to have both of these tools' response 
      //       types store two sets of coordinates: "instant" coordinates that we don't scale 
      //       (and that the tool itself doesn't scale by frame time), and "non-instant" 
      //       coordinates that do get scaled (both here and by the tool itself).
      //

      //
      // NOTE: We no longer normalize the movement direction and then apply a speed to scale 
      //       it by. I expect this to be friendlier to gamepad inputs (not yet tested), but 
      //       it means that "strafe-running" now happens: for keyboard use, diagonal inputs 
      //       are faster than cardinal. (Joysticks should be normalized.)
      // 
      //       I think that for Hold inputs, we should check if the movement vector has a 
      //       length greater than 1, and only if so, we should normalize it. We can do that 
      //       during the "scale" step, before we scale by the frame time.
      //

      worldedit_core._adjust_camera({}, update);
   }
}