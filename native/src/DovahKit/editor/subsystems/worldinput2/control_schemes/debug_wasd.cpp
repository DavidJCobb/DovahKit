#include "./reach.h"
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
   using worldedit::pointer_position_type;
   using worldedit::reference_frame;
   using worldedit::selection_operation;
   using worldedit::sign;
}

namespace dovahkit::subsystems::worldinput2::default_control_schemes {
   // Must be a getter to avoid the static initialization order fiasco, which 
   // affects my Win32 API wrappers
   extern binds::tree& debug_wasd() {
      static auto out = binds::tree(input_device_type::keyboard_mouse);
      static bool initialized = false;
      if (initialized)
         return out;
      initialized = true;

      {  // Move Camera
         struct _bind {
            const char* name;
            wchar_t     key;
            float x = 0;
            float y = 0;
            float z = 0;
         };
         constexpr auto binds = std::array{
            _bind{ ("Move Camera Forward"), 'W',  0,  1,  0 },
            _bind{ ("Move Camera Back"),    'S',  0, -1,  0 },
            _bind{ ("Move Camera Left"),    'A', -1,  0,  0 },
            _bind{ ("Move Camera Right"),   'D',  1,  0,  0 },
            _bind{ ("Move Camera Up"),      'Q',  0,  0,  1 },
            _bind{ ("Move Camera Down"),    'Z',  0,  0, -1 },
         };
         for (const auto& item : binds) {
            auto* node = new binds::nodes::bound_tool;
            node->name              = item.name;
            node->button_press_type = button_press_type::hold;
            out.root->append(*node);

            auto* g = node->input_sequence.root = new input_sequence::group;
            g->type   = input_sequence::group_type::single_control;
            g->button = inputs::button{ .key = cobb::keyboard::key::from_character(item.key, false) };

            node->tool.id      = tools::id_of<tools::move_camera>;
            node->tool.options = new tools::options_union(tools::move_camera::options{
               .reference_frames = {
                  .baseline  = reference_frame::camera,
                  .selection = reference_frame::camera,
               },
               .magnitudes = { .x = item.x, .y = item.y, .z = item.z },
            });
         }
      }
      {  // Turn Camera
         struct _bind {
            const char* name;
            wchar_t     key;
            float pitch = 0;
            float yaw   = 0; // clockwise, so turning left is negative
         };
         constexpr auto binds = std::array{
               _bind{ "Turn Camera Left",  'G',  0, -1},
               _bind{ "Turn Camera Right", 'H',  0,  1},
               _bind{ "Turn Camera Up",    'R',  1,  0},
               _bind{ "Turn Camera Down" , 'V', -1,  0},
         };
         for (const auto& item : binds) {
            auto* node = new binds::nodes::bound_tool;
            node->name              = item.name;
            node->button_press_type = button_press_type::hold;
            out.root->append(*node);

            auto* g = node->input_sequence.root = new input_sequence::group;
            g->type   = input_sequence::group_type::single_control;
            g->button = inputs::button{ .key = cobb::keyboard::key::from_character(item.key, false) };

            node->tool.id      = tools::id_of<tools::turn_camera>;
            node->tool.options = new tools::options_union(tools::turn_camera::options{
               .magnitudes = { .yaw = item.yaw, .pitch = item.pitch },
            });
         }
      }
      {  // Boost
         auto* node = new binds::nodes::bound_tool;
         node->name              = "Boost";
         node->button_press_type = button_press_type::hold;
         out.root->append(*node);

         auto* g = node->input_sequence.root = new input_sequence::group;
         g->type   = input_sequence::group_type::single_control;
         g->button = inputs::button{ .key = cobb::keyboard::key(cobb::keyboard::virtual_key::shift) };

         node->tool.id      = tools::id_of<tools::modify_camera_speed_flags>;
         node->tool.options = new tools::options_union(tools::modify_camera_speed_flags::options{
            .boost = bool_operation::set_true,
         });
      }
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

            node->tool.id      = tools::id_of<tools::attempt_on_screen_selection>;
            node->tool.options = new tools::options_union(tools::attempt_on_screen_selection::options{
               .operation = selection_operation::replace,
               .position  = pointer_position_type::mouse,
            });
         }
         {  // Replace Selection
            auto* node = new binds::nodes::bound_tool;
            node->name              = "Replace Selection";
            node->button_press_type = button_press_type::press;
            em_node->append(*node);

            node->input_sequence = algorithms::input_sequence_from_string("[Ctrl + LMB]");

            node->tool.id      = tools::id_of<tools::attempt_on_screen_selection>;
            node->tool.options = new tools::options_union(tools::attempt_on_screen_selection::options{
               .operation = selection_operation::toggle,
               .position  = pointer_position_type::mouse,
            });
         }
      }
      return out;
   }
}