#pragma once
#include "../warning.h"
namespace dovah::datastores::impl::idles2 {
   class idle_node;
}

namespace dovah::datastores::impl::idles2::warnings {
   //
   // Idle and its intended previous siblings have inconsistent parents, so the 
   // idle and its next-siblings will be loaded as loose idles.
   //
   class siblings_have_mismatched_parents : public warning {
      public:
         siblings_have_mismatched_parents(idle_node& s) : subject(s) {};

      public:
         idle_node& subject;
   };
}