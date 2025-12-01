#pragma once
#include "../warning.h"
namespace dovah::datastores::impl::idles2 {
   class idle_node;
}

namespace dovah::datastores::impl::idles2::warnings {
   //
   // Idle has been placed as both an action root and a child idle, and so may 
   // end up in multiple places at once.
   //
   class child_idle_is_an_action_root_candidate : public warning {
      public:
         child_idle_is_an_action_root_candidate(idle_node& s) : subject(s) {};

      public:
         idle_node& subject;
   };
}