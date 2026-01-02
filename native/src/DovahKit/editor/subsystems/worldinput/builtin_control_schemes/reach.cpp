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
   using worldedit::camera_orbit_target;
   using worldedit::camera_turn_axis;
   using worldedit::editor_mode;
   using worldedit::gizmo_mode;
   using worldedit::reference_frame;
   using worldedit::selection_operation;
   using worldedit::sign;

   using range_input_scales    = worldinput::util::range_input_scales;
   using range_input_scales_1D = worldinput::util::range_input_scales_1D;

   using worldinput::condition_set;

   using worldinput::control_scheme;
   using worldinput::control_scheme_action;
   using worldinput::control_scheme_condition;
   using worldinput::control_scheme_modifier;
   using control_scheme_node = worldinput::control_scheme::node;
}

namespace dovahkit::subsystems::worldinput::builtin_control_schemes {
   extern const control_scheme& reach() {
      static auto out = control_scheme(input_device_type::xinput);
      static bool initialized = false;
      if (initialized)
         return out;
      initialized = true;

      {
         auto* node_no_selections = control_scheme_node::from_data(control_scheme_condition{
            .name = "When nothing is selected...",
            .data = {
               .selection_count = condition_set::selection_count_comparison_set{
                  .comparisons = {
                     { comparison_operator::less_or_equal, 0 },
                  }
               }
            },
         });
         out.top_level_nodes.push_back(node_no_selections);
         
         node_no_selections->append_child(*control_scheme_node::from_data(control_scheme_action{
            .name = "Move Camera Laterally",
            //
            .input_sequence    = algorithms::input_sequence_from_string(":: Left Stick"),
            .button_press_type = button_press_type::hold,
            //
            .tool = _tool_with_options(tools::move_camera::options{
               .frame      = reference_frame::camera,
               .magnitudes = { 1, 1, 0 },
               .range = range_input_scales{
                  .x = { axis3D::x, sign::positive },
                  .y = { axis3D::y, sign::positive },
               },
            })
         }));
         node_no_selections->append_child(*control_scheme_node::from_data(control_scheme_action{
            .name = "Move Camera Down",
            //
            .input_sequence    = _single_button_sequence(inputs::xinput_button::lb),
            .button_press_type = button_press_type::hold,
            //
            .tool = _tool_with_options(tools::move_camera::options{
               .frame      = reference_frame::camera,
               .magnitudes = { 0, 0, -1 },
            })
         }));
         node_no_selections->append_child(*control_scheme_node::from_data(control_scheme_action{
            .name = "Move Camera Up",
            //
            .input_sequence    = _single_button_sequence(inputs::xinput_button::rb),
            .button_press_type = button_press_type::hold,

            .tool = _tool_with_options(tools::move_camera::options{
               .frame      = reference_frame::camera,
               .magnitudes = { 0, 0, 1 },
            })
         }));
         node_no_selections->append_child(*control_scheme_node::from_data(control_scheme_action{
            .name = "Turn Camera",
            //
            .input_sequence    = algorithms::input_sequence_from_string(":: Right Stick"),
            .button_press_type = button_press_type::hold,
            //
            .tool = _tool_with_options(tools::turn_camera::options{
               .magnitudes = { 1, 0, 1 },
               .range = range_input_scales{
                  .x = { axis3D::z, sign::positive },
                  .y = { axis3D::x, sign::positive },
               },
            })
         }));
      }
      {
         auto* node_selections = control_scheme_node::from_data(control_scheme_condition{
            .name = "When anything is selected...",
            .data = {
               .selection_count = condition_set::selection_count_comparison_set{
                  .comparisons = {
                     { comparison_operator::greater, 0 },
                  }
               }
            },
         });
         out.top_level_nodes.push_back(node_selections);
         
         node_selections->append_child(*control_scheme_node::from_data(control_scheme_action{
            .name = "Move Cam and Selection Laterally",
            //
            .input_sequence    = algorithms::input_sequence_from_string(":: Left Stick"),
            .button_press_type = button_press_type::hold,
            //
            .tool = _tool_with_options(tools::move_selection::options{
               .frame = reference_frame::camera,
               .also_move_camera = true,
               .magnitudes = { 1, 1, 0 },
               .range = range_input_scales{
                  .x = { axis3D::x, sign::positive },
                  .y = { axis3D::y, sign::positive },
               },
               .locked_axes = {
                  .frame = reference_frame::world,
                  .z = true,
               },
            })
         }));
         node_selections->append_child(*control_scheme_node::from_data(control_scheme_action{
            .name = "Move Cam and Selection Down",
            //
            .input_sequence    = _single_button_sequence(inputs::xinput_button::lb),
            .button_press_type = button_press_type::hold,
            //
            .tool = _tool_with_options(tools::move_selection::options{
               .frame = reference_frame::world,
               .also_move_camera = true,
               .magnitudes = { 0, 0, -1 },
            })
         }));
         node_selections->append_child(*control_scheme_node::from_data(control_scheme_action{
            .name = "Move Cam and Selection Up",
            //
            .input_sequence    = _single_button_sequence(inputs::xinput_button::rb),
            .button_press_type = button_press_type::hold,

            .tool = _tool_with_options(tools::move_selection::options{
               .frame = reference_frame::world,
               .also_move_camera = true,
               .magnitudes = { 0, 0, 1 },
            })
         }));
         node_selections->append_child(*control_scheme_node::from_data(control_scheme_action{
            .name = "Orbit Camera",
            //
            .input_sequence    = algorithms::input_sequence_from_string(":: Right Stick"),
            .button_press_type = button_press_type::hold,
            //
            .tool = _tool_with_options(tools::orbit_camera::options{
               .magnitudes = { 1, 0, 1 },
               .range = range_input_scales{
                  .x = { axis3D::z, sign::positive },
                  .y = { axis3D::x, sign::positive },
               },
               .target = camera_orbit_target::primary_selection,
            })
         }));

         {
            auto* modifier = control_scheme_node::from_data(control_scheme_modifier{
               .name = "Right Trigger",
               .input_sequence = _single_button_sequence(inputs::xinput_button::rt),
            });
            node_selections->append_child(*modifier);
            
            modifier->append_child(*control_scheme_node::from_data(control_scheme_action{
               .name = "Zoom",
               //
               .input_sequence    = algorithms::input_sequence_from_string(":: Left Stick Y"),
               .button_press_type = button_press_type::hold,
               //
               .tool = _tool_with_options(tools::move_camera::options{
                  .magnitudes = { 0, 1, 0 },
                  .range = range_input_scales{
                     .x = { axis3D::x, sign::positive },
                     .y = { axis3D::y, sign::positive },
                  },
               })
            }));
            //
            // TOOD: Rotate selection on `:: Left Stick X`
            //

            modifier->append_child(*control_scheme_node::from_data(control_scheme_action{
               .name = "Scale",
               //
               .input_sequence    = algorithms::input_sequence_from_string("LT :: Left Stick", true),
               .button_press_type = button_press_type::hold,
               //
               .tool = _tool_with_options(tools::scale_selection::options{
                  .mod                = 0.5F,
                  .scale_all_together = true,
                  //
                  .range = range_input_scales_1D{
                     .x = sign::positive,
                     .y = sign::positive,
                  },
               })
            }));
         }
      }
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
               .input_sequence    = _single_button_sequence(inputs::xinput_button::a),
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
      {
         auto* em_node = control_scheme_node::from_data(control_scheme_condition{
            .name = "Object Mode",
            .data = {
               .editor_modes = editor_mode::objects,
            }
         });
         out.top_level_nodes.push_back(em_node);
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
            em_node->append_child(*node);

            auto& is = node->data.input_sequence;
            is.raycast.associated_button = is.root;
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