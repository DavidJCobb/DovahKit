#pragma once
#include "../warning.h"
namespace dovah::datastores::impl::idles {
   class idle_node;
}

namespace dovah::datastores::impl::idles::warnings {
   //
   // Idle has ended up orphaned, seemingly not belonging to any behavior 
   // graph.
   //
   class orphaned_idle : public warning {
      public:
         orphaned_idle(idle_node& s) : subject(s) {};

      public:
         idle_node& subject;
   };
}