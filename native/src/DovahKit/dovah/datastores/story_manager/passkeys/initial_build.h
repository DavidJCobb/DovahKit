#pragma once
namespace dovah::datastores {
   class story_manager;
}

namespace dovah::datastores::impl::story_manager::passkeys {
   class initial_build {
      friend ::dovah::datastores::story_manager;
      private:
         constexpr initial_build() {}
   };
}
