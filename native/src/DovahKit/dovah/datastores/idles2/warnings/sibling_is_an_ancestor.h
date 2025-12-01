#pragma once
#include "../warning.h"
namespace dovah::datastores::impl::idles2 {
   class idle_node;
}

namespace dovah::datastores::impl::idles2::warnings {
   //
   // Idle defines a degenerate hierarchy placement (its intended parent is one 
   // of its intended previous-siblings), and has therefore been loaded as a 
   // loose idle.
   //
   class sibling_is_an_ancestor : public warning {
      public:
         sibling_is_an_ancestor(idle_node& s) : subject(s) {};

      public:
         idle_node& subject;
   };
}