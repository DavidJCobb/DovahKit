#include "./ck_standard.h"
#include "./_builders.h"
#include "editor/subsystems/worldedit/tool_system/options_union.h"

namespace {
   namespace worldedit {
      using namespace dovahkit::subsystems::worldedit;
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
}

namespace dovahkit::subsystems::worldinput2::default_control_schemes {
   // Must be a getter to avoid the static initialization order fiasco, which 
   // affects my Win32 API wrappers
   extern binds::tree& ck_standard() {
      static auto out = binds::tree(input_device_type::keyboard_mouse);
      static bool initialized = false;
      if (initialized)
         return out;
      initialized = true;
      
      out.root->append(*build::tool_node(
         "Turn Camera",
         button_press_type::hold,
         "Shift :: Mouse Move",
         worldedit::tools::turn_camera::options{
            .magnitudes = { 1, 1 },
            .range = {
               .x = { camera_turn_axis::yaw,   sign::positive },
               .y = { camera_turn_axis::pitch, sign::positive },
            },
         }
      ));
      out.root->append(*build::tool_node(
         "Move Camera",
         button_press_type::hold,
         "MMB :: Mouse Move",
         worldedit::tools::move_camera::options{
            .reference_frames = {
               .baseline  = reference_frame::camera,
               .selection = reference_frame::camera,
            },
            .magnitudes = { 1, 0, 1 },
            .range = {
               .x = { axis3D::x, sign::positive },
               .y = { axis3D::y, sign::positive },
            },
         }
      ));
      //
      // TODO: "Move Camera" bind on camera Y-axis, mapped to scroll wheel
      //
      out.root->append(*build::tool_node( // Creation Kit: Turn Camera takes priority over Move Camera (Worldinput rules mean the above two would just block each other)
         "Turn Camera (override Move Camera)",
         button_press_type::hold,
         "(Shift + MMB) :: Mouse Move",
         worldedit::tools::turn_camera::options{
            .magnitudes = { 1, 1 },
            .range = {
               .x = { camera_turn_axis::yaw,   sign::positive },
               .y = { camera_turn_axis::pitch, sign::positive },
            },
         }
      ));
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
            out.root->append(*build::tool_node(
               item.name,
               button_press_type::hold,
               cobb::keyboard::key::from_character(item.key, false),
               tools::set_edit_gizmo_mode::options{
                  .gizmo = {
                     .a = item.mode,
                     .b = gizmo_mode::none,
                  },
                  .toggle_gizmo = item.toggle,
                  .modify_gizmo = true,
               }
            ));
         }
      }
      out.root->append(*build::tool_node(
         "Toggle Gizmo Reference Frame (World/Local)",
         button_press_type::hold,
         cobb::keyboard::key::from_character('G', false),
         tools::set_edit_gizmo_mode::options{
            .frame = {
               .a = reference_frame::world,
               .b = reference_frame::local,
            },
            .toggle_frame = true,
         }
      ));
      {  // Editor Mode: Objects
         auto* em_node = new binds::nodes::editor_mode(editor_mode::objects);
         out.root->append(*em_node);

         {  // Replace Selection
            auto* node = new binds::nodes::bound_tool;
            node->name              = "Replace Selection";
            node->button_press_type = button_press_type::press;
            em_node->append(*node);

            auto* g = node->input_sequence.root = new input_sequence::group;
            g->type   = input_sequence::group_type::single_control;
            g->button = inputs::button{ .mouse = Qt::MouseButton::LeftButton };
            
            node->input_sequence.raycast.associated_button = g;
            node->input_sequence.raycast.requirement = raycast_requirement{
               .targets = {
                  .object_references = true,
               },
            };

            node->tool.id      = tools::id_of<tools::attempt_on_screen_selection>;
            node->tool.options = new tools::options_union(tools::attempt_on_screen_selection::options{
               .operation = selection_operation::replace,
            });
         }
         {  // Replace Selection
            auto* node = new binds::nodes::bound_tool;
            node->name              = "Replace Selection";
            node->button_press_type = button_press_type::press;
            em_node->append(*node);

            node->input_sequence = algorithms::input_sequence_from_string("[Ctrl + LMB]");
            auto* lmb = node->input_sequence.root->children[1];
            
            node->input_sequence.raycast.associated_button = lmb;
            node->input_sequence.raycast.requirement = raycast_requirement{
               .targets = {
                  .object_references = true,
               },
            };

            node->tool.id      = tools::id_of<tools::attempt_on_screen_selection>;
            node->tool.options = new tools::options_union(tools::attempt_on_screen_selection::options{
               .operation = selection_operation::toggle,
            });
         }
      }
      return out;
   }
}