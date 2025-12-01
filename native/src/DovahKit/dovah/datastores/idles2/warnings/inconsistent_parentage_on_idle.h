#pragma once
#include "../warning.h"
namespace dovah::datastores::impl::idles2 {
   class idle_node;
}

namespace dovah::datastores::impl::idles2::warnings {
   //
   // This idle is not present in its parent's list of children.
   //
   class inconsistent_parentage_on_idle : public warning {
      public:
         inconsistent_parentage_on_idle(idle_node& s) : subject(s) {};

      public:
         idle_node& subject;
   };
}