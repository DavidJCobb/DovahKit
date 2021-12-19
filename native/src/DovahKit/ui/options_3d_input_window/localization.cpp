#include "localization.h"
#include "dk3d/tools/_all.h"

/*static*/ QString DK3DLocalization::stringify_input(const DK3D::inputs::bound_input& bi) {
   if (bi.is_button()) {
      QString mod;
      switch (bi.button.press_type) {
         using _ = DK3D::button_press_type;
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
      } else if (bi.button.gamepad != DK3D::inputs::xinput_button::None) {
         switch (bi.button.gamepad) {
            using _ = DK3D::inputs::xinput_button;
            case _::A:
               button = tr("A", "XInput button");
               break;
            case _::B:
               button = tr("B", "XInput button");
               break;
            case _::X:
               button = tr("X", "XInput button");
               break;
            case _::Y:
               button = tr("Y", "XInput button");
               break;
            case _::LT:
               button = tr("LT", "XInput button");
               break;
            case _::RT:
               button = tr("RT", "XInput button");
               break;
            case _::LB:
               button = tr("LB", "XInput button");
               break;
            case _::RB:
               button = tr("RB", "XInput button");
               break;
            case _::LS:
               button = tr("LS-Click", "XInput button");
               break;
            case _::RS:
               button = tr("RS-Click", "XInput button");
               break;
            case _::DPadUp:
               button = tr("D-Pad Up", "XInput button");
               break;
            case _::DPadDown:
               button = tr("D-Pad Down", "XInput button");
               break;
            case _::DPadLeft:
               button = tr("D-Pad Left", "XInput button");
               break;
            case _::DPadRight:
               button = tr("D-Pad Right", "XInput button");
               break;
            case _::Back:
               button = tr("Back", "XInput button");
               break;
            case _::Start:
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
         using _ = DK3D::scalar_control;
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
            using _ = DK3D::axis2D;
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
         using _ = DK3D::vector_control;
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
/*static*/ QString DK3DLocalization::tool_name(DK3D::tool_id id) {
switch (id) {
   case DK3D::tools::id_of_none:
      return tr("None", "tool name");
      //
   case DK3D::id_of_tool<DK3D::tools::debug_log>():
      return tr("Debug Log", "tool name");
   case DK3D::id_of_tool<DK3D::tools::debug_placeholder>():
      return tr("Placeholder", "tool name");
   case DK3D::id_of_tool<DK3D::tools::move_camera>():
      return tr("Move Camera", "tool name");
   case DK3D::id_of_tool<DK3D::tools::turn_camera>():
      return tr("Turn Camera", "tool name");
}
return tr("<unknown>", "tool name");
}
/*static*/ QString DK3DLocalization::tool_name(const DK3D::tools::base* tool) {
   if (!tool)
      return tool_name(DK3D::tools::id_of_none);
   return tool_name(DK3D::all_tool_instances::get().id_of(*tool));
}