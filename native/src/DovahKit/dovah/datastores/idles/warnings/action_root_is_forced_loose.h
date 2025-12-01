#pragma once
#include "../warning.h"
namespace dovah::datastores::impl::idles {
   class idle_node;
}

namespace dovah::datastores::impl::idles::warnings {
   //
   // Idle is an action root, but is also flagged as loose, and so risks 
   // ending up in multiple places at once.
   //
   class action_root_is_forced_loose : public warning {
      public:
         action_root_is_forced_loose(idle_node& s) : subject(s) {};

      public:
         idle_node& subject;
   };
}