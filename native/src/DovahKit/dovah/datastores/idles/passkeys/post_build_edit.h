#pragma once
namespace dovah::datastores {
   class idles;
}

namespace dovah::datastores::impl::idles::passkeys {
   class post_build_edit {
      friend ::dovah::datastores::idles;
      private:
         constexpr post_build_edit() {}
   };
}
