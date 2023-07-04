#include "./worldinput2_input_seq_ui.h"

#include "editor/subsystems/worldinput2/algorithms/input_sequence_stringification.h"
#include "editor/subsystems/worldinput2/control_scheme/action.h"

#include "ui/options_window/worldinput_bind_editor.h"

#include "editor/subsystems/worldedit/tool_system/id_of.h"
#include "editor/subsystems/worldedit/tool_system/options_union.h"
#include "editor/subsystems/worldedit/tool_system/tools/modify_camera_speed_flags.h"

namespace {
   namespace worldedit {
      using namespace dovahkit::subsystems::worldedit;
   }
   namespace worldinput2 {
      using namespace dovahkit::subsystems::worldinput2;
   }
}

namespace DovahKitDebug::features {
   /*static*/ void worldinput2_input_seq_ui::execute(QWidget* from) {

      auto data = worldinput2::control_scheme_action{
         .name = "Temporary Test Node",
         //
         .input_sequence     = worldinput2::algorithms::input_sequence_from_string("<A + S + D + [<J + K> + Z]>"),
         .button_press_type = worldinput2::button_press_type::press,
      };
      //
      data.input_sequence.range.control = worldinput2::range_input_control::mouse_move;
      data.input_sequence.range.axes    = worldinput2::range_input_axes::y;
      //
      data.input_sequence.raycast.associated_button = data.input_sequence.root->children[1]; // 'S'
      data.input_sequence.raycast.requirement.targets.object_references = true;

      if constexpr (true) { // tool test
         using namespace dovahkit::subsystems::worldedit;

         data.tool.id      = tools::id_of<tools::modify_camera_speed_flags>;
         data.tool.options = new tools::options_union;

         auto* ou = (tools::options_union*)data.tool.options;
         *ou = tools::options_union(tools::modify_camera_speed_flags::options{
            .boost     = bool_operation::set_true,
            .precision = bool_operation::set_false,
         });
      }

      auto* dialog = new WorldinputBindEditDialog(worldinput2::input_device_type::keyboard_mouse, from);
      dialog->initializeFrom(data);
      dialog->exec();

      dialog->overwrite(data);
      delete dialog;

      #if _DEBUG
         __debugbreak(); // Inspect `node` to confirm any changes made were properly set
      #endif
   }
}