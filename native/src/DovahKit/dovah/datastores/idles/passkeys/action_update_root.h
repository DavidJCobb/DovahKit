#pragma once
namespace dovah::datastores::impl::idles {
   class action_node;
}

namespace dovah::datastores::impl::idles::passkeys {
   class action_update_root {
      friend action_node;
      private:
         constexpr action_update_root() {}
   };
}