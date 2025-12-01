#pragma once
namespace dovah::datastores {
   class idles2;
}

namespace dovah::datastores::impl::idles2::passkeys {
   class initial_build {
      friend ::dovah::datastores::idles2;
      private:
         constexpr initial_build() {}
   };
}
