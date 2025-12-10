#pragma once
namespace dovah::datastores {
   class camera_paths;
}

namespace dovah::datastores::impl::camera_paths::passkeys {
   class post_build_edit {
      friend ::dovah::datastores::camera_paths;
      private:
         constexpr post_build_edit() {}
   };
}
