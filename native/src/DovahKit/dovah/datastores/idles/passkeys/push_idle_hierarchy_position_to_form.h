#pragma once
namespace dovah::datastores {
   class idles;
}
namespace dovah::datastores::impl::idles {
   class idle_parent_node;
   class idle_node;
}

namespace dovah::datastores::impl::idles::passkeys {
   class push_idle_hierarchy_position_to_form {
      friend ::dovah::datastores::idles;
      friend idle_parent_node;
      friend idle_node;
      private:
         constexpr push_idle_hierarchy_position_to_form() {}
   };
}