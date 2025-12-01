#pragma once
namespace dovah::datastores {
   class idles;
}

namespace dovah::datastores::impl::idles::passkeys {
   class initial_build {
      friend ::dovah::datastores::idles;
      private:
         constexpr initial_build() {}
   };
}
