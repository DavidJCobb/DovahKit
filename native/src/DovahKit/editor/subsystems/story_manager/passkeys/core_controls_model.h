#pragma once
namespace dovahkit::subsystems::story_manager {
   class core;
}

namespace dovahkit::subsystems::story_manager::passkeys {
   class core_controls_model {
      friend core;
      private:
         constexpr core_controls_model() {}
   };
}