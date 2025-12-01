#pragma once
namespace dovah::datastores::impl::idles {
   class action_node;
}

namespace dovah::datastores::impl::idles::passkeys {
   class fully_delete_action {
      friend action_node;
      private:
         constexpr fully_delete_action() {}
   };
}