#pragma once
#include "./_base.h"
namespace dovah::datastores::impl::idles {
   class idle_node;
}

namespace dovah::datastores::impl::idles::warnings {
   class idle_has_multiple_next_siblings : public warning {
      public:
         constexpr idle_has_multiple_next_siblings(
            const idle_node& i
         ) :
            idle(i)
         {}

      public:
         const idle_node& idle;
   };
}