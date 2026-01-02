#pragma once
namespace dovahkit::subsystems::worldedit::tools {
   class attempt_on_screen_pick_ref;
}

namespace dovahkit::subsystems::worldedit::passkeys {
   class attempt_pick_ref {
      friend tools::attempt_on_screen_pick_ref;
      private:
         constexpr attempt_pick_ref() {}
   };
}