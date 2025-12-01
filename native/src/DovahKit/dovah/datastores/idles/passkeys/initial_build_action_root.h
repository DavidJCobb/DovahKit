#pragma once
namespace dovah::datastores {
   class idles;
}
namespace dovah::datastores::impl::idles {
   class action_node;
}

namespace dovah::datastores::impl::idles::passkeys {
   class initial_build_action_root {
      friend ::dovah::datastores::idles;
      friend action_node;
      private:
         constexpr initial_build_action_root() {}
   };
}