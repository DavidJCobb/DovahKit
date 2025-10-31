#pragma once
#include "./_base.h"
namespace dovah::datastores::impl::idles {
   class idle_node;
}

namespace dovah::datastores::impl::idles::warnings {
   // Idle has no parent idle, but also isn't an action root.
   class orphaned_idle : public warning {
      public:
         constexpr orphaned_idle(const idle_node& i) : idle(i) {}

      public:
         const idle_node& idle;
   };
}