#pragma once
#include "../warning.h"
namespace dovah::datastores::impl::idles2 {
   class idle_node;
}

namespace dovah::datastores::impl::idles2::warnings {
   //
   // Idle has ended up loose, but wasn't originally flagged as loose. Is this 
   // intentional?
   //
   class loose_idle_is_not_flagged : public warning {
      public:
         loose_idle_is_not_flagged(idle_node& s) : subject(s) {};

      public:
         idle_node& subject;
   };
}