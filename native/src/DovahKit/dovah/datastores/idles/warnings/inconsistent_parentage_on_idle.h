#pragma once
#include "../warning.h"
namespace dovah::datastores::impl::idles {
   class idle_node;
}

namespace dovah::datastores::impl::idles::warnings {
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