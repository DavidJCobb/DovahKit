#pragma once
namespace dovah::datastores {
   class story_manager;
}

namespace dovah::datastores::impl::story_manager::passkeys {
   class post_build_edit {
      friend ::dovah::datastores::story_manager;
      private:
         constexpr post_build_edit() {}
   };
}
