#pragma once
#include "../warning.h"
namespace dovah::datastores::impl::idles2 {
   class idle_node;
}

namespace dovah::datastores::impl::idles2::warnings {
   //
   // No loose idle list for this idle.
   //
   class no_loose_idle_list_for_idle : public warning {
      public:
         no_loose_idle_list_for_idle(idle_node& s) : subject(s) {};

      public:
         idle_node& subject;
   };
}