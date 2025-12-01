#pragma once
namespace dovah::datastores::impl::idles {
   class action_node;
   class idle_node;
}

namespace dovah::datastores::impl::idles::passkeys {
   class check_is_clearing {
      friend action_node;
      friend idle_node;
      private:
         constexpr check_is_clearing() {}
   };
}