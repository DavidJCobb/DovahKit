#pragma once
#include "../warning.h"
namespace dovah::datastores::impl::idles {
   class idle_node;
}

namespace dovah::datastores::impl::idles::warnings {
   //
   // Idle is part of a cyclical hierarchy (via its parent), and has therefore 
   // been loaded as a loose idle.
   //
   class cyclical_parent_relationships : public warning {
      public:
         cyclical_parent_relationships(idle_node& s) : subject(s) {};

      public:
         idle_node& subject;
   };
}