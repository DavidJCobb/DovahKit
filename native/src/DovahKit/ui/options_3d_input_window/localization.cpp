#include "localization.h"
#include "editor/subsystems/worldinput/tools/_all.h"

using namespace dovahkit::subsystems::worldinput;

/*static*/ QString DKWorldinputLocalization::stringify_input(const inputs::bound_input& bi) {
   if (bi.is_button()) {
      QString mod;
      switch (bi.button.press_type) {
         using _ = button_press_type;
         case _::tap:
            mod = tr("Tap", "boolean input mod");
            break;
         case _::hold:
            mod = tr("Hold", "boolean input mod");
            break;
         case _::while_down:
            mod = tr("While", "boolean input mod");
            break;
      }
      QString button;
      if (!bi.button.key.empty()) {
         button = bi.button.key.toString();
      } else if (bi.button.gamepad != inputs::xinput_button::none) {
         switch (bi.button.gamepad) {
            using _ = inputs::xinput_button;
            case _::a:
               button = tr("A", "XInput button");
               break;
            case _::b:
               button = tr("B", "XInput button");
               break;
            case _::x:
               button = tr("X", "XInput button");
               break;
            case _::y:
               button = tr("Y", "XInput button");
               break;
            case _::trigger_left:
               button = tr("LT", "XInput button");
               break;
            case _::trigger_right:
               button = tr("RT", "XInput button");
               break;
            case _::bumper_left:
               button = tr("LB", "XInput button");
               break;
            case _::bumper_right:
               button = tr("RB", "XInput button");
               break;
            case _::stick_click_left:
               button = tr("LS-Click", "XInput button");
               break;
            case _::stick_click_right:
               button = tr("RS-Click", "XInput button");
               break;
            case _::d_pad_up:
               button = tr("D-Pad Up", "XInput button");
               break;
            case _::d_pad_down:
               button = tr("D-Pad Down", "XInput button");
               break;
            case _::d_pad_left:
               button = tr("D-Pad Left", "XInput button");
               break;
            case _::d_pad_right:
               button = tr("D-Pad Right", "XInput button");
               break;
            case _::back:
               button = tr("Back", "XInput button");
               break;
            case _::start:
               button = tr("Start", "XInput button");
               break;
         }
      } else if (bi.button.mouse != Qt::MouseButton::NoButton) {
         switch (bi.button.mouse) {
            using _ = Qt::MouseButton;
            case _::LeftButton:
               button = tr("LMB", "mouse button");
               break;
            case _::RightButton:
               button = tr("RMB", "mouse button");
               break;
            case _::MiddleButton:
               button = tr("MMB", "mouse button");
               break;
            case _::XButton1:
               button = tr("Mouse-X1", "mouse button");
               break;
            case _::XButton2:
               button = tr("Mouse-X2", "mouse button");
               break;
         }
      }
      //
      return tr("%1 %2", "format string for button inputs").arg(mod).arg(button);
   }
   if (bi.is_scalar()) {
      QString control;
      //
      bool has_axis = false;
      switch (bi.scalar.input) {
         using _ = scalar_control;
         case _::mouse_move:
            control  = tr("Mouse-Move", "scalar control");
            has_axis = true;
            break;
         case _::xinput_ls:
            control  = tr("LS", "scalar control");
            has_axis = true;
            break;
         case _::xinput_rs:
            control  = tr("RS", "scalar control");
            has_axis = true;
            break;
         case _::xinput_lt:
            control = tr("LT", "scalar control");
            break;
         case _::xinput_rt:
            control = tr("RT", "scalar control");
            break;
      }
      if (has_axis) {
         QString axis;
         switch (bi.scalar.axis) {
            using _ = axis2D;
            case _::x:
               axis = tr("Left/Right", "scalar axis");
               break;
            case _::y:
               axis = tr("Up/Down", "scalar axis");
               break;
         }
         if (!axis.isEmpty())
            return tr("%1 %2", "scalar input format string").arg(control).arg(axis);
      }
      return control;
   }
   if (bi.is_vector()) {
      QString control;
      switch (bi.vector.input) {
         using _ = vector_control;
         case _::mouse_move:
            return tr("Mouse-Move", "vector control");
         case _::xinput_ls:
            return tr("LS", "vector control");
         case _::xinput_rs:
            return tr("RS", "vector control");
      }
   }
   return tr("<none>");
}
/*static*/ QString DKWorldinputLocalization::tool_name(tool_id id) {
   using namespace dovahkit::subsystems::worldinput;

   switch (id) {
      case tools::id_of_none:
         return tr("None", "tool name");
         //
      case id_of_tool<tools::attempt_on_screen_selection>():
         return tr("Try Select ObjectReference", "tool name");
      case id_of_tool<tools::debug_log>():
         return tr("Debug Log", "tool name");
      case id_of_tool<tools::debug_placeholder>():
         return tr("Debug Placeholder", "tool name");
      case id_of_tool<tools::modify_camera_speed_flags>():
         return tr("Set Camera Speed Mode", "tool name");
      case id_of_tool<tools::move_camera>():
         return tr("Move Camera", "tool name");
      case id_of_tool<tools::turn_camera>():
         return tr("Turn Camera", "tool name");
   }
   return tr("<unknown>", "tool name");
}
/*static*/ QString DKWorldinputLocalization::tool_name(const tools::base* tool) {
   if (!tool)
      return tool_name(tools::id_of_none);
   return tool_name(all_tool_instances::get().id_of(*tool));
}