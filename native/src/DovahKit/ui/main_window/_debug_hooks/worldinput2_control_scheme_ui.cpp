#include "./worldinput2_control_scheme_ui.h"


#include "editor/subsystems/worldinput2/builtin_control_schemes/debug_wasd.h"
#include "editor/subsystems/worldinput2/control_scheme.h"

#include "ui/options_window/worldinput_scheme_editor.h"

namespace {
   namespace worldinput {
      using namespace dovahkit::subsystems::worldinput;
   }
}

namespace DovahKitDebug::features {
   /*static*/ void worldinput2_control_scheme_ui::execute(QWidget* from) {

      // intentional copy
      worldinput::control_scheme tree = worldinput::builtin_control_schemes::debug_wasd();
      tree.name = "Temporary Test Control Scheme";

      auto* dialog = new WorldinputSchemeEditDialog(worldinput::input_device_type::keyboard_mouse, from);
      dialog->initializeFrom(tree);
      auto result = dialog->exec();

      if (result == QDialog::DialogCode::Accepted) {
         dialog->overwrite(tree);
      }
      delete dialog;

      #if _DEBUG
         __debugbreak(); // Inspect `tree` to confirm any changes made were properly set
      #endif
   }
}