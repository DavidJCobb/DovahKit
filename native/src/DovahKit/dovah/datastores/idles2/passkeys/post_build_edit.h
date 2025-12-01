#pragma once
namespace dovah::datastores {
   class idles2;
}

namespace dovah::datastores::impl::idles2::passkeys {
   class post_build_edit {
      friend ::dovah::datastores::idles2;
      private:
         constexpr post_build_edit() {}
   };
}
