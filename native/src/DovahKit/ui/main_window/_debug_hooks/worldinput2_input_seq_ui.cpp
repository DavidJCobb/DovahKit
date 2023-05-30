#include "./worldinput2_input_seq_ui.h"

#include "editor/subsystems/worldinput2/algorithms/input_sequence_stringification.h"
#include "editor/subsystems/worldinput2/bind_tree/nodes/bound_tool.h"

#include "ui/options_window/worldinput_bind_editor.h"

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

      auto* node = new worldinput2::binds::nodes::bound_tool{};
      node->name              = "Temporary Test Node";
      node->button_press_type = worldinput2::button_press_type::press;
      node->input_sequence    = worldinput2::algorithms::input_sequence_from_string("<A + S + D + [<J + K> + Z]>");

      node->input_sequence.range.control = worldinput2::range_input_control::mouse_move;
      node->input_sequence.range.axes    = worldinput2::range_input_axes::y;

      node->input_sequence.raycast.associated_button = node->input_sequence.root->children[1]; // 'S'
      node->input_sequence.raycast.requirement.targets.object_references = true;

      auto* dialog = new WorldinputBindEditDialog(worldinput2::input_device_type::keyboard_mouse, from);
      dialog->setAttribute(Qt::WA_DeleteOnClose);
      dialog->initializeFrom(*node);
      dialog->show();
   }
}