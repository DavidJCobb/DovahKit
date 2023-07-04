#include "./worldinput2_control_scheme_ui.h"


#include "editor/subsystems/worldinput2/builtin_control_schemes/debug_wasd.h"
#include "editor/subsystems/worldinput2/control_scheme.h"

#include "ui/options_window/worldinput_scheme_editor.h"

namespace {
   namespace worldinput2 {
      using namespace dovahkit::subsystems::worldinput2;
   }
}

namespace DovahKitDebug::features {
   /*static*/ void worldinput2_control_scheme_ui::execute(QWidget* from) {

      // intentional copy
      worldinput2::control_scheme tree = dovahkit::subsystems::worldinput2::builtin_control_schemes::debug_wasd();
      tree.name = "Temporary Test Control Scheme";

      auto* dialog = new WorldinputSchemeEditDialog(worldinput2::input_device_type::keyboard_mouse, from);
      dialog->initializeFrom(tree);
      dialog->exec();

      dialog->overwrite(tree);
      delete dialog;

      #if _DEBUG
         __debugbreak(); // Inspect `tree` to confirm any changes made were properly set
      #endif
   }
}