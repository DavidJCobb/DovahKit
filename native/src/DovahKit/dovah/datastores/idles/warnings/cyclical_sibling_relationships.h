#pragma once
#include "./_base.h"
namespace dovah::datastores::impl::idles {
   class idle_node;
}

namespace dovah::datastores::impl::idles::warnings {
   class cyclical_sibling_relationships : public warning {
      public:
         constexpr cyclical_sibling_relationships(const idle_node& i) : idle(i) {}

      public:
         const idle_node& idle;
   };
}