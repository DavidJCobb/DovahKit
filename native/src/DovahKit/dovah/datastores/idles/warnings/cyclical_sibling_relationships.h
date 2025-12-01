#pragma once
#include "../warning.h"
namespace dovah::datastores::impl::idles {
   class idle_node;
}

namespace dovah::datastores::impl::idles::warnings {
   //
   // Idle is part of a cyclical hierarchy (via its previous sibling), and has 
   // therefore been loaded as a loose idle.
   //
   class cyclical_sibling_relationships : public warning {
      public:
         cyclical_sibling_relationships(idle_node& s) : subject(s) {};

      public:
         idle_node& subject;
   };
}