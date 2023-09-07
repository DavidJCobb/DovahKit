#include "./worldinput2_control_scheme_rebuild.h"
#include "editor/subsystems/worldedit/tool_system/id_of.h"
#include "editor/subsystems/worldedit/tool_system/options_union.h"
#include "editor/subsystems/worldinput2/algorithms/control_scheme_to_bind_list.h"
#include "editor/subsystems/worldinput2/algorithms/input_sequence_stringification.h"
#include "editor/subsystems/worldinput2/control_scheme/all_node_headers.h"
#include "editor/subsystems/worldinput2/control_scheme.h"

#include "helpers/bitstreams/reader.h"
#include "helpers/bitstreams/writer.h"

namespace {
   namespace worldedit {
      using namespace dovahkit::subsystems::worldedit;
   }
   namespace worldinput {
      using namespace dovahkit::subsystems::worldinput;
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

   static worldinput::input_sequence _seq_from_key(const cobb::keyboard::key& key) {
      using namespace worldinput;

      input_sequence out;

      auto* g = out.root = new input_sequence::group;
      g->type = input_sequence::group_type::single_control;
      g->button.key = key;

      return out;
   }
   static worldinput::input_sequence _seq_from_key(Qt::MouseButton mb) {
      using namespace worldinput;

      input_sequence out;

      auto* g = out.root = new input_sequence::group;
      g->type = input_sequence::group_type::single_control;
      g->button.mouse = mb;

      return out;
   }

   template<typename Tool>
   static auto _tool_sans_options() {
      decltype(worldinput::control_scheme_action::tool) out = {};
      out.id = worldedit::tools::id_of<Tool>;
      return out;
   }

   template<typename Options>
   static auto _tool_with_options(const Options& options) {
      decltype(worldinput::control_scheme_action::tool) out = {};
      out.id      = worldedit::tools::id_of<Options>;
      out.options = new tools::options_union(options);
      return out;
   }
   
   static worldinput::control_scheme& make_scheme() {
      using worldinput::control_scheme;
      using worldinput::control_scheme_action;
      using worldinput::control_scheme_condition;
      using worldinput::control_scheme_condition_node;
      using worldinput::control_scheme_modifier;
      using worldinput::button_press_type;
      using worldinput::input_device_type;
      using worldinput::raycast_requirement;

      static auto out = control_scheme(input_device_type::keyboard_mouse);
      static bool initialized = false;
      if (initialized)
         return out;
      initialized = true;

      {
         auto data = control_scheme_action{
            .name = QString("Debug: Dump Raycast"),
            //
            .input_sequence    = _seq_from_key(cobb::keyboard::key(cobb::keyboard::virtual_key::numpad_0)),
            .button_press_type = button_press_type::press,
            //
            .tool = _tool_sans_options<worldedit::tools::debug_dump_raycast>(),
         };
         data.input_sequence.raycast.requirement = raycast_requirement{
            .targets = {
               .edit_gizmo_mode   = gizmo_mode::translate,
               .landscapes        = true,
               .nothing           = true,
               .object_references = true,
            },
         };
         data.input_sequence.raycast.associated_button = data.input_sequence.root;

         out.top_level_nodes.push_back(control_scheme::node::make<decltype(data)>(data));
      }
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
            auto data = control_scheme_action{
               .name = item.name,
               //
               .input_sequence    = _seq_from_key(cobb::keyboard::key::from_character(item.key, false)),
               .button_press_type = button_press_type::hold,
               //
               .tool = _tool_with_options(tools::move_camera::options{
                  .reference_frames = {
                     .baseline  = reference_frame::camera,
                     .selection = reference_frame::camera,
                  },
                  .magnitudes = { .x = item.x, .y = item.y, .z = item.z },
               }),
            };

            out.top_level_nodes.push_back(control_scheme::node::make<decltype(data)>(data));
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
            auto data = control_scheme_action{
               .name = item.name,
               //
               .input_sequence    = _seq_from_key(cobb::keyboard::key::from_character(item.key, false)),
               .button_press_type = button_press_type::hold,
               //
               .tool = _tool_with_options(tools::turn_camera::options{
                  .magnitudes = { .yaw = item.yaw, .pitch = item.pitch },
               }),
            };

            out.top_level_nodes.push_back(control_scheme::node::make<decltype(data)>(data));
         }
      }
      {  // Boost
         auto data = control_scheme_action{
            .name = "Boost",
            //
            .input_sequence    = _seq_from_key(cobb::keyboard::key(cobb::keyboard::virtual_key::shift)),
            .button_press_type = button_press_type::hold,
            //
            .tool = _tool_with_options(tools::modify_camera_speed_flags::options{
               .boost = bool_operation::set_true,
            }),
         };

         out.top_level_nodes.push_back(control_scheme::node::make<decltype(data)>(data));
      }
      {  // Edit Gizmo modes
         struct _bind {
            const char* name;
            cobb::keyboard::virtual_key vk;
            gizmo_mode  mode;
            bool        toggle = false;
         };
         constexpr auto binds = std::array{
            _bind{ "Show Translate Gizmo", cobb::keyboard::virtual_key::numpad_1, gizmo_mode::translate, true },
            _bind{ "Toggle Rotate Gizmo",  cobb::keyboard::virtual_key::numpad_2, gizmo_mode::rotate,    true },
            _bind{ "Show Scale Gizmo",     cobb::keyboard::virtual_key::numpad_3, gizmo_mode::scale,     true },
         };
         for (const auto& item : binds) {
            auto data = control_scheme_action{
               .name = item.name,
               //
               .input_sequence    = _seq_from_key(cobb::keyboard::key(item.vk)),
               .button_press_type = button_press_type::press,
               //
               .tool = _tool_with_options(tools::set_edit_gizmo_mode::options{
                  .gizmo = {
                     .a = item.mode,
                     .b = gizmo_mode::none,
                  },
                  .toggle_gizmo = item.toggle,
                  .modify_gizmo = true,
               }),
            };

            out.top_level_nodes.push_back(control_scheme::node::make<decltype(data)>(data));
         }
      }
      {  // Editor Mode: Objects
         auto* em_node = control_scheme::node::make<control_scheme_condition_node>(control_scheme_condition_node{
            .name = "Object Mode",
            .data = {
               .editor_modes = editor_mode::objects,
            }
         });
         out.top_level_nodes.push_back(em_node);

         {  // Replace Selection
            auto data = control_scheme_action{
               .name = "Replace Selection",
               //
               .input_sequence    = _seq_from_key(Qt::MouseButton::LeftButton),
               .button_press_type = button_press_type::press,
               //
               .tool = _tool_with_options(tools::attempt_on_screen_selection::options{
                  .operation = selection_operation::replace,
               }),
            };
            data.input_sequence.raycast = {
               .associated_button = data.input_sequence.root,
               .requirement       = raycast_requirement{
                  .targets = {
                     .object_references = true,
                  },
               }
            };

            em_node->append_child(*control_scheme::node::make<decltype(data)>(data));
         }
         {  // Toggle Selection
            auto data = control_scheme_action{
               .name = "Toggle Selection",
               //
               .input_sequence    = worldinput::algorithms::input_sequence_from_string("[Ctrl + LMB]"),
               .button_press_type = button_press_type::press,
               //
               .tool = _tool_with_options(tools::attempt_on_screen_selection::options{
                  .operation = selection_operation::toggle,
               }),
            };
            data.input_sequence.raycast = {
               .associated_button = data.input_sequence.root->children[1],
               .requirement       = raycast_requirement{
                  .targets = {
                     .object_references = true,
                  },
               }
            };

            em_node->append_child(*control_scheme::node::make<decltype(data)>(data));
         }
      }
      return out;
   }
}

namespace DovahKitDebug::features {
   /*static*/ void worldinput2_control_scheme_rebuild::execute(QWidget* from) {

      worldinput::control_scheme scheme = make_scheme();
      scheme.name = "Temporary Test Control Scheme";

      auto flattened = worldinput::algorithms::control_scheme_to_bind_list(scheme);

      #if _DEBUG
         __debugbreak();
      #endif
         
      auto _compare = [](const char* name, const worldinput::control_scheme& src) {
         cobb::bitstreams::writer writer;
         src.write(writer);

         cobb::bitstreams::reader reader;
         reader.set_buffer(writer.data(), writer.get_bytespan());
         const auto dst = worldinput::control_scheme::read(reader);

         bool equal = (src == dst);

         qDebug("%s serialization worked? %s", name, equal ? "true" : "false");
         #if _DEBUG
            if (!equal) {
               __debugbreak();
            }
         #endif
      };
      _compare("Debug WASD", scheme);
      qDebug("All tests run.");
   }
}