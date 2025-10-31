#pragma once
namespace dovah::datastores {
   class idles;
}
namespace dovah::datastores::impl::idles {
   class idle_parent_node;
}

namespace dovah::datastores::impl::idles::passkeys {
   class idle_sorting {
      friend ::dovah::datastores::idles;
      friend idle_parent_node;
      private:
         constexpr idle_sorting() {}
   };
}