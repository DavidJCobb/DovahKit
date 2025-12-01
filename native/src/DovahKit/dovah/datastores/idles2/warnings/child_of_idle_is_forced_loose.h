#pragma once
#include "../warning.h"
namespace dovah::datastores::impl::idles2 {
   class idle_node;
}

namespace dovah::datastores::impl::idles2::warnings {
   //
   // Idle is set to be the child of another idle, but is also flagged as 
   // loose, so it will not in fact be a child. Is this intentional?
   //
   class child_of_idle_is_forced_loose : public warning {
      public:
         child_of_idle_is_forced_loose(idle_node& s) : subject(s) {};

      public:
         idle_node& subject;
   };
}