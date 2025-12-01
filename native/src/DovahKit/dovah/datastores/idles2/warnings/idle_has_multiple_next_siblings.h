#pragma once
#include "../warning.h"
namespace dovah::datastores::impl::idles2 {
   class idle_node;
}

namespace dovah::datastores::impl::idles2::warnings {
   //
   // Multiple idles are competing to be this idle's next-sibling.
   //
   class idle_has_multiple_next_siblings : public warning {
      public:
         idle_has_multiple_next_siblings(idle_node& s) : subject(s) {};

      public:
         idle_node& subject;
   };
}
