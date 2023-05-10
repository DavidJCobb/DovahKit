#include "./reach.h"
#include "./_builders.h"
#include "editor/subsystems/worldedit/tool_system/options_union.h"

namespace {
   namespace worldedit {
      using namespace dovahkit::subsystems::worldedit;
   }
   using worldedit::axis3D;
   using worldedit::bool_operation;
   using worldedit::camera_turn_axis;
   using worldedit::editor_mode;
   using worldedit::reference_frame;
   using worldedit::selection_operation;
   using worldedit::sign;
}

namespace dovahkit::subsystems::worldinput2::default_control_schemes {
   extern binds::tree& reach() {
      static auto out = binds::tree(input_device_type::xinput);
      static bool initialized = false;
      if (initialized)
         return out;
      initialized = true;

      out.root->append(*build::tool_node(
         "Move Camera Laterally",
         button_press_type::hold,
         ":: Left Stick",
         worldedit::tools::move_camera::options{
            .reference_frames = {
               .baseline  = reference_frame::camera,
               .selection = reference_frame::camera,
            },
            .magnitudes = { 1, 1, 0 },
            .range = {
               .x = { axis3D::x, sign::positive },
               .y = { axis3D::y, sign::positive },
            },
         }
      ));
      out.root->append(*build::tool_node(
         "Move Camera Down",
         button_press_type::hold,
         inputs::xinput_button::lb,
         worldedit::tools::move_camera::options{
            .reference_frames = {
               .baseline  = reference_frame::camera,
               .selection = reference_frame::camera,
            },
            .magnitudes = { 0, 0, -1 },
         }
      ));
      out.root->append(*build::tool_node(
         "Move Camera Up",
         button_press_type::hold,
         inputs::xinput_button::rb,
         worldedit::tools::move_camera::options{
            .reference_frames = {
               .baseline  = reference_frame::camera,
               .selection = reference_frame::camera,
            },
            .magnitudes = { 0, 0, 1 },
         }
      ));
      out.root->append(*build::tool_node(
         "Turn Camera",
         button_press_type::hold,
         ":: Right Stick",
         worldedit::tools::turn_camera::options{
            .magnitudes = { 1, 1 },
            .range = {
               .x = { camera_turn_axis::yaw,   sign::positive },
               .y = { camera_turn_axis::pitch, sign::positive },
            },
         }
      ));
      out.root->append(*build::tool_node(
         "Boost",
         button_press_type::hold,
         inputs::xinput_button::lt,
         worldedit::tools::modify_camera_speed_flags::options{
            .boost = bool_operation::set_true,
         }
      ));
      out.root->append(*build::tool_node(
         "Toggle Precision",
         button_press_type::press,
         inputs::xinput_button::lt,
         worldedit::tools::modify_camera_speed_flags::options{
            .precision = bool_operation::invert,
         }
      ));

      {
         auto* node = build::tool_node(
            "Toggle Selection",
            button_press_type::press,
            inputs::xinput_button::a,
            worldedit::tools::attempt_on_screen_selection::options{
               .operation = selection_operation::toggle,
            }
         );
         node->input_sequence.raycast.associated_button = node->input_sequence.root;
         node->input_sequence.raycast.requirement = raycast_requirement{
            .targets = {
               .object_references = true,
            },
         };
      }

      return out;
   }
}