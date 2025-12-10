#pragma once
namespace dovah::datastores {
   class camera_paths;
}

namespace dovah::datastores::impl::camera_paths::passkeys {
   class initial_build {
      friend ::dovah::datastores::camera_paths;
      private:
         constexpr initial_build() {}
   };
}
