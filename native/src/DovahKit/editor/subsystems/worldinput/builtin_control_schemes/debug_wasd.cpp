#include "./ck_standard.h"
#include "./_helpers.h"
#include "../algorithms/input_sequence_stringification.h"
#include "../control_scheme/all_node_headers.h"

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

   using range_input_scales = worldinput::util::range_input_scales;

   using worldinput::condition_set;

   using worldinput::control_scheme;
   using worldinput::control_scheme_action;
   using worldinput::control_scheme_condition;
   using worldinput::control_scheme_modifier;
   using control_scheme_node = worldinput::control_scheme::node;
}

namespace dovahkit::subsystems::worldinput::builtin_control_schemes {
   // Must be a getter to avoid the static initialization order fiasco, which 
   // affects my Win32 API wrappers
   extern const control_scheme& debug_wasd() {
      static auto out = control_scheme(input_device_type::keyboard_mouse);
      static bool initialized = false;
      if (initialized)
         return out;
      initialized = true;
      
      {
         auto* node = control_scheme_node::from_data(control_scheme_action{
            .name = "Debug: Dump Raycast",
            //
            .input_sequence    = _single_button_sequence(cobb::keyboard::key(cobb::keyboard::virtual_key::numpad_0)),
            .button_press_type = button_press_type::press,
            //
            .tool = _tool_sans_options<worldedit::tools::debug_dump_raycast>(),
         });

         auto& is = node->data.input_sequence;
         is.raycast.requirement = raycast_requirement{
            .targets = {
               .edit_gizmo_mode   = gizmo_mode::translate,
               .landscapes        = true,
               .nothing           = true,
               .object_references = true,
            },
         };
         is.raycast.associated_button = is.root;

         out.top_level_nodes.push_back(node);
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
            out.top_level_nodes.push_back(control_scheme_node::from_data(control_scheme_action{
               .name = item.name,
               //
               .input_sequence    = _single_button_sequence(cobb::keyboard::key::from_character(item.key, false)),
               .button_press_type = button_press_type::hold,
               //
               .tool = _tool_with_options(tools::move_camera::options{
                  .frame      = reference_frame::camera,
                  .magnitudes = { item.x, item.y, item.z },
               })
            }));
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
            out.top_level_nodes.push_back(control_scheme_node::from_data(control_scheme_action{
               .name = item.name,
               //
               .input_sequence    = _single_button_sequence(cobb::keyboard::key::from_character(item.key, false)),
               .button_press_type = button_press_type::hold,
               //
               .tool = _tool_with_options(tools::turn_camera::options{
                  .magnitudes = { item.pitch, 0, item.yaw },
               })
            }));
         }
      }
      {  // Boost
         out.top_level_nodes.push_back(control_scheme_node::from_data(control_scheme_action{
            .name = "Boost",
            //
            .input_sequence    = _single_button_sequence(cobb::keyboard::key(cobb::keyboard::virtual_key::shift)),
            .button_press_type = button_press_type::hold,
            //
            .tool = _tool_with_options(tools::modify_camera_speed_flags::options{
               .boost = bool_operation::set_true,
            })
         }));
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
            out.top_level_nodes.push_back(control_scheme_node::from_data(control_scheme_action{
               .name = item.name,
               //
               .input_sequence    = _single_button_sequence(cobb::keyboard::key(item.vk)),
               .button_press_type = button_press_type::press,
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
      {  // If any objects are selected...
         auto* node_selections = control_scheme_node::from_data(control_scheme_condition{
            .name = "When anything is selected...",
            .data = {
               .editor_modes    = decltype(condition_set::editor_modes)::value_type::from<editor_mode::objects, editor_mode::navmesh>(),
               .selection_count = condition_set::selection_count_comparison_set{
                  .comparisons = {
                     { comparison_operator::greater, 0 },
                  }
               }
            },
         });
         out.top_level_nodes.push_back(node_selections);

         #pragma region Transformation by steps, based on gizmo
         {
            auto* node_translate = control_scheme_node::from_data(control_scheme_condition{
               .name = "Translate by steps",
               .data = {
                  .gizmo_modes = gizmo_mode::translate,
               }
            });
            auto* node_rotate = control_scheme_node::from_data(control_scheme_condition{
               .name = "Rotate by steps",
               .data = {
                  .gizmo_modes = gizmo_mode::rotate,
               }
            });
            auto* node_scale = control_scheme_node::from_data(control_scheme_condition{
               .name = "Scale by steps",
               .data = {
                  .gizmo_modes = gizmo_mode::scale,
               }
            });
            node_selections->append_child(*node_translate);
            node_selections->append_child(*node_rotate);
            node_selections->append_child(*node_scale);

            auto seq_x_inc = algorithms::input_sequence_from_string("Numpad 7");
            auto seq_x_dec = algorithms::input_sequence_from_string("Numpad 4");
            auto seq_y_inc = algorithms::input_sequence_from_string("Numpad 8");
            auto seq_y_dec = algorithms::input_sequence_from_string("Numpad 5");
            auto seq_z_inc = algorithms::input_sequence_from_string("Numpad 9");
            auto seq_z_dec = algorithms::input_sequence_from_string("Numpad 6");

            for (int i = 0; i < 3; ++i) {
               axis3D axis = axis3D::x;
               switch (i) {
                  case 1: axis = axis3D::y; break;
                  case 2: axis = axis3D::z; break;
               }

               cobb::vector3<float> magnitudes_inc;
               cobb::vector3<float> magnitudes_dec;
               magnitudes_inc[i] = 32.0;
               magnitudes_dec[i] = -32.0;

               QString name_inc = "Increase, ";
               name_inc += (char)(i + 'X');
               name_inc += "-Axis";

               QString name_dec = "Decrease, ";
               name_dec += (char)(i + 'X');
               name_dec += "-Axis";

               input_sequence seq_inc;
               input_sequence seq_dec;
               switch (axis) {
                  case axis3D::x:
                     seq_inc = seq_x_inc;
                     seq_dec = seq_x_dec;
                     break;
                  case axis3D::y:
                     seq_inc = seq_y_inc;
                     seq_dec = seq_y_dec;
                     break;
                  case axis3D::z:
                     seq_inc = seq_z_inc;
                     seq_dec = seq_z_dec;
                     break;
               }
               
               node_translate->append_child(*control_scheme_node::from_data(control_scheme_action{
                  .name              = name_inc,
                  .input_sequence    = seq_inc,
                  .button_press_type = button_press_type::press,
                  //
                  .tool = _tool_with_options(tools::move_selection::options{
                     .frame      = reference_frame::current,
                     .magnitudes = magnitudes_inc,
                  })
               }));
               node_translate->append_child(*control_scheme_node::from_data(control_scheme_action{
                  .name              = name_dec,
                  .input_sequence    = seq_dec,
                  .button_press_type = button_press_type::press,
                  //
                  .tool = _tool_with_options(tools::move_selection::options{
                     .frame      = reference_frame::current,
                     .magnitudes = magnitudes_dec,
                  })
               }));
               //
               // TODO: rotate
               // TODO: scale
               //
            }
         }
         #pragma endregion

         {  // Translate Gizmo drag
            for (int i = 0; i < 3; ++i) {
               axis3D axis = axis3D::x;
               switch (i) {
                  case 1: axis = axis3D::y; break;
                  case 2: axis = axis3D::z; break;
               }

               QString name = "Drag Translate Gizmo, ";
               name += (char)(i + 'X');
               name += "-Axis";

               auto* node = control_scheme_node::from_data(control_scheme_action{
                  .name = name,
                  //
                  .input_sequence    = algorithms::input_sequence_from_string("LMB :: Mouse Move"),
                  .button_press_type = button_press_type::hold,
                  //
                  .tool = _tool_with_options(tools::move_selection_by_drag::options{
                     .frame    = reference_frame::current,
                     .axis     = axis,
                     .is_plane = false,
                  })
               });
               node_selections->append_child(*node);

               auto& is = node->data.input_sequence;
               is.raycast.associated_button = is.root;
               is.raycast.requirement = raycast_requirement{
                  .targets = {
                     .edit_gizmo_axis = axis,
                     .edit_gizmo_mode = gizmo_mode::translate,
                  },
               };
            }
         }
      }
      {  // Editor Mode: Pick Ref
         auto* em_node = control_scheme_node::from_data(control_scheme_condition{
            .name = "When picking a ref",
            .data = {
               .editor_modes = editor_mode::picking_ref,
            }
         });
         out.top_level_nodes.push_back(em_node);
         
         {  // Pick Ref
            auto* node = control_scheme_node::from_data(control_scheme_action{
               .name = "Pick ref",
               //
               .button_press_type = button_press_type::press,
               //
               .tool = _tool_sans_options<tools::attempt_on_screen_pick_ref>()
            });
            em_node->append_child(*node);

            auto& data = node->data;

            auto* g = data.input_sequence.root = new input_sequence::group;
            g->type   = input_sequence::group_type::single_control;
            g->button = inputs::button{ .mouse = Qt::MouseButton::LeftButton };
         }
      }
      {  // Editor Mode: Objects
         auto* em_node = control_scheme_node::from_data(control_scheme_condition{
            .name = "Object Mode",
            .data = {
               .editor_modes = editor_mode::objects,
            }
         });
         out.top_level_nodes.push_back(em_node);

         {  // Replace Selection
            auto* node = control_scheme_node::from_data(control_scheme_action{
               .name = "Replace Selection",
               //
               .input_sequence    = algorithms::input_sequence_from_string("LMB"),
               .button_press_type = button_press_type::press,
               //
               .tool = _tool_with_options(tools::attempt_on_screen_selection::options{
                  .operation = selection_operation::replace,
               })
            });
            em_node->append_child(*node);

            auto& is = node->data.input_sequence;
            is.raycast.associated_button = is.root;
            is.raycast.requirement = raycast_requirement{
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

            auto& is = node->data.input_sequence;
            is.raycast.associated_button = is.root->children[1]; // LMB
            is.raycast.requirement = raycast_requirement{
               .targets = {
                  .object_references = true,
               },
            };
         }
      }

      out.assert_validity();

      return out;
   }
}