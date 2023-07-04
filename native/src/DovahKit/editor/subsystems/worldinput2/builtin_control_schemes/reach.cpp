#include "./ck_standard.h"
#include "./_helpers.h"
#include "../algorithms/input_sequence_stringification.h"
#include "../control_scheme/all_node_headers.h"

namespace {
   namespace worldedit {
      using namespace dovahkit::subsystems::worldedit;
   }
   namespace worldinput2 {
      using namespace dovahkit::subsystems::worldinput2;
   }
   namespace tools {
      using namespace dovahkit::subsystems::worldedit::tools;
   }
   using worldedit::axis3D;
   using worldedit::bool_operation;
   using worldedit::camera_turn_axis;
   using worldedit::editor_mode;
   using worldedit::gizmo_mode;
   using worldedit::reference_frame;
   using worldedit::selection_operation;
   using worldedit::sign;

   using worldinput2::control_scheme;
   using worldinput2::control_scheme_action;
   using worldinput2::control_scheme_condition;
   using worldinput2::control_scheme_modifier;
   using control_scheme_node = worldinput2::control_scheme::node;
}

namespace dovahkit::subsystems::worldinput2::builtin_control_schemes {
   extern const control_scheme& reach() {
      static auto out = control_scheme(input_device_type::xinput);
      static bool initialized = false;
      if (initialized)
         return out;
      initialized = true;

      out.top_level_nodes.push_back(control_scheme_node::from_data(control_scheme_action{
         .name = "Move Camera Laterally",
         //
         .input_sequence    = algorithms::input_sequence_from_string(":: Left Stick"),
         .button_press_type = button_press_type::hold,
         //
         .tool = _tool_with_options(tools::move_camera::options{
            .reference_frames = {
               .baseline = reference_frame::camera,
               .selection = reference_frame::camera,
            },
            .magnitudes = { 1, 1, 0 },
            .range = {
               .x = { axis3D::x, sign::positive },
               .y = { axis3D::y, sign::positive },
            },
         })
      }));
      out.top_level_nodes.push_back(control_scheme_node::from_data(control_scheme_action{
         .name = "Move Camera Down",
         //
         .input_sequence    = _single_button_sequence(inputs::xinput_button::lb),
         .button_press_type = button_press_type::hold,
         //
         .tool = _tool_with_options(tools::move_camera::options{
            .reference_frames = {
               .baseline  = reference_frame::camera,
               .selection = reference_frame::camera,
            },
            .magnitudes = { 0, 0, -1 },
         })
      }));
      out.top_level_nodes.push_back(control_scheme_node::from_data(control_scheme_action{
         .name = "Move Camera Up",
         //
         .input_sequence    = _single_button_sequence(inputs::xinput_button::rb),
         .button_press_type = button_press_type::hold,

         .tool = _tool_with_options(tools::move_camera::options{
            .reference_frames = {
               .baseline  = reference_frame::camera,
               .selection = reference_frame::camera,
            },
            .magnitudes = { 0, 0, 1 },
         })
      }));
      out.top_level_nodes.push_back(control_scheme_node::from_data(control_scheme_action{
         .name = "Turn Camera",
         //
         .input_sequence    = algorithms::input_sequence_from_string(":: Right Stick"),
         .button_press_type = button_press_type::hold,
         //
         .tool = _tool_with_options(tools::turn_camera::options{
            .magnitudes = { 1, 1 },
            .range = {
               .x = { camera_turn_axis::yaw,   sign::positive },
               .y = { camera_turn_axis::pitch, sign::positive },
            },
         })
      }));
      out.top_level_nodes.push_back(control_scheme_node::from_data(control_scheme_action{
         .name = "Boost",
         //
         .input_sequence    = _single_button_sequence(inputs::xinput_button::lt),
         .button_press_type = button_press_type::hold,
         //
         .tool = _tool_with_options(tools::modify_camera_speed_flags::options{
            .boost = bool_operation::set_true,
         })
      }));
      out.top_level_nodes.push_back(control_scheme_node::from_data(control_scheme_action{
         .name = "Toggle Precision",
         //
         .input_sequence    = _single_button_sequence(inputs::xinput_button::ls),
         .button_press_type = button_press_type::press,
         //
         .tool = _tool_with_options(tools::modify_camera_speed_flags::options{
            .precision = bool_operation::invert,
         })
      }));

      {
         auto* node = control_scheme_node::from_data(control_scheme_action{
            .name = "Toggle Selection",
            //
            .input_sequence    = _single_button_sequence(inputs::xinput_button::a),
            .button_press_type = button_press_type::press,
            //
            .tool = _tool_with_options(tools::attempt_on_screen_selection::options{
               .operation = selection_operation::toggle,
            })
         });
         out.top_level_nodes.push_back(node);

         auto& is = node->data.input_sequence;
         is.raycast.associated_button = is.root;
         is.raycast.requirement = raycast_requirement{
            .targets = {
               .object_references = true,
            },
         };
      }

      return out;
   }
}