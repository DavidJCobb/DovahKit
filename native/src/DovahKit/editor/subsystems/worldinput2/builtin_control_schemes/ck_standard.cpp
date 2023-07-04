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
   // Must be a getter to avoid the static initialization order fiasco, which 
   // affects my Win32 API wrappers
   extern const control_scheme& ck_standard() {
      static auto out = control_scheme(input_device_type::keyboard_mouse);
      static bool initialized = false;
      if (initialized)
         return out;
      initialized = true;
      
      out.top_level_nodes.push_back(control_scheme_node::from_data(control_scheme_action{
         .name = "Turn Camera",
         //
         .input_sequence    = algorithms::input_sequence_from_string("Shift :: Mouse Move"),
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
         .name = "Move Camera",
         //
         .input_sequence    = algorithms::input_sequence_from_string("MMB :: Mouse Move"),
         .button_press_type = button_press_type::hold,
         //
         .tool = _tool_with_options(worldedit::tools::move_camera::options{
            .reference_frames = {
               .baseline  = reference_frame::camera,
               .selection = reference_frame::camera,
            },
            .magnitudes = { 1, 0, 1 },
            .range = {
               .x = { axis3D::x, sign::positive },
               .y = { axis3D::y, sign::positive },
            },
         })
      }));
      //
      // TODO: "Move Camera" bind on camera Y-axis, mapped to scroll wheel
      //
      out.top_level_nodes.push_back(control_scheme_node::from_data(control_scheme_action{ // Creation Kit: Turn Camera takes priority over Move Camera (Worldinput rules mean the above two would just block each other)
         .name = "Turn Camera (override Move Camera)",
         //
         .input_sequence    = algorithms::input_sequence_from_string("(Shift + MMB) :: Mouse Move"),
         .button_press_type = button_press_type::hold,
         //
         .tool = _tool_with_options(worldedit::tools::turn_camera::options{
            .magnitudes = { 1, 1 },
            .range = {
               .x = { camera_turn_axis::yaw,   sign::positive },
               .y = { camera_turn_axis::pitch, sign::positive },
            },
         })
      }));
      {  // Edit Gizmo modes
         struct _bind {
            const char* name;
            wchar_t     key;
            gizmo_mode  mode;
            bool        toggle = false;
         };
         constexpr auto binds = std::array{
            _bind{ "Show Scale Gizmo",     '2', gizmo_mode::scale,     true },
            _bind{ "Show Translate Gizmo", 'E', gizmo_mode::translate, true },
            _bind{ "Hide Gizmo",           'R', gizmo_mode::none,      false },
            _bind{ "Toggle Rotate Gizmo",  'W', gizmo_mode::rotate,    true },
         };
         for (const auto& item : binds) {
            out.top_level_nodes.push_back(control_scheme_node::from_data(control_scheme_action{
               .name = item.name,
               //
               .input_sequence    = _single_button_sequence(cobb::keyboard::key::from_character(item.key, false)),
               .button_press_type = button_press_type::hold,
               //
               .tool = _tool_with_options(tools::set_edit_gizmo_mode::options{
                  .gizmo = {
                     .a = item.mode,
                     .b = gizmo_mode::none,
                  },
                  .toggle_gizmo = item.toggle,
                  .modify_gizmo = true,
               })
            }));
         }
      }
      out.top_level_nodes.push_back(control_scheme_node::from_data(control_scheme_action{
         .name = "Toggle Gizmo Reference Frame (World/Local)",
         //
         .input_sequence    = _single_button_sequence(cobb::keyboard::key::from_character('G', false)),
         .button_press_type = button_press_type::hold,
         //
         .tool = _tool_with_options(tools::set_edit_gizmo_mode::options{
            .frame = {
               .a = reference_frame::world,
               .b = reference_frame::local,
            },
            .toggle_frame = true,
         })
      }));
      {  // Editor Mode: Objects
         auto* em_node = control_scheme_node::from_data(control_scheme_condition{
            .mode = editor_mode::objects,
         });
         out.top_level_nodes.push_back(em_node);

         {  // Replace Selection
            auto* node = control_scheme_node::from_data(control_scheme_action{
               .name = "Replace Selection",
               //
               .button_press_type = button_press_type::press,
               //
               .tool = _tool_with_options(tools::attempt_on_screen_selection::options{
                  .operation = selection_operation::replace,
               })
            });
            em_node->append_child(*node);

            auto& data = node->data;

            auto* g = data.input_sequence.root = new input_sequence::group;
            g->type   = input_sequence::group_type::single_control;
            g->button = inputs::button{ .mouse = Qt::MouseButton::LeftButton };
            
            data.input_sequence.raycast.associated_button = g;
            data.input_sequence.raycast.requirement = raycast_requirement{
               .targets = {
                  .object_references = true,
               },
            };
         }
         {  // Toggle Selection
            auto* node = control_scheme_node::from_data(control_scheme_action{
               .name = "Toggle Selection",
               //
               .input_sequence    = algorithms::input_sequence_from_string("[Ctrl + LMB]"),
               .button_press_type = button_press_type::press,
               //
               .tool = _tool_with_options(tools::attempt_on_screen_selection::options{
                  .operation = selection_operation::toggle,
               })
            });
            em_node->append_child(*node);

            auto& data = node->data;

            auto* lmb = data.input_sequence.root->children[1];
            data.input_sequence.raycast.associated_button = lmb;
            data.input_sequence.raycast.requirement = raycast_requirement{
               .targets = {
                  .object_references = true,
               },
            };
         }
      }
      return out;
   }
}