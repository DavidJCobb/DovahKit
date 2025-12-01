#pragma once
namespace dovah::datastores::impl::idles {
   class idle_node;
}

namespace dovah::datastores::impl::idles::passkeys {
   class fully_delete_idle {
      friend idle_node;
      private:
         constexpr fully_delete_idle() {}
   };
}