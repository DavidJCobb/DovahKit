#pragma once
#include "../warning.h"
namespace dovah::datastores::impl::idles2 {
   class idle_node;
}

namespace dovah::datastores::impl::idles2::warnings {
   //
   // An idle attempts to be the root idle for multiple actions, and may 
   // end up being in multiple places at once.
   //
   // This can happen if an override attempts to re-parent an action root. 
   // More rarely, it could happen if a malformed IDLE record contains 
   // multiple ANAM subrecords placing the same idle in different action 
   // roots.
   //
   class idle_has_candidacies_for_multiple_actions : public warning {
      public:
         idle_has_candidacies_for_multiple_actions(idle_node& s) : subject(s) {};

      public:
         idle_node& subject;
   };
}