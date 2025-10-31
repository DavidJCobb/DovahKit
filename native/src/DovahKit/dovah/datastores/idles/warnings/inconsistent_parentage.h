#pragma once
#include "./_base.h"
namespace dovah::datastores::impl::idles {
   class idle_node;
}

namespace dovah::datastores::impl::idles::warnings {
   // CK sanity check, for whether an idle's parent contains that idle.
   class inconsistent_parentage : public warning {
      public:
         constexpr inconsistent_parentage(const idle_node& i) : idle(i) {}

      public:
         const idle_node& idle;
   };
}