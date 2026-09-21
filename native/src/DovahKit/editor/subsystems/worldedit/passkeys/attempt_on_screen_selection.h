#pragma once
namespace dovahkit::subsystems::worldedit::tools {
   class attempt_on_screen_selection;
}

namespace dovahkit::subsystems::worldedit::passkeys {
   class attempt_on_screen_selection {
      friend tools::attempt_on_screen_selection;
      private:
         constexpr attempt_on_screen_selection() {}
   };
}