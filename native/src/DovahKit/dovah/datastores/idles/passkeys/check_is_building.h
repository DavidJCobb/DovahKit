#pragma once
namespace dovah::datastores {
   class idles;
}
namespace dovah::datastores::impl::idles {
   class action_parent_node;
   class idle_parent_node;
}

namespace dovah::datastores::impl::idles::passkeys {
   class check_is_building {
      friend ::dovah::datastores::idles;
      friend action_parent_node;
      friend idle_parent_node;
      private:
         constexpr check_is_building() {}
   };
}