#pragma once
#include "./_base.h"
namespace dovah::datastores::impl::idles {
   class idle_node;
}

namespace dovah::datastores::impl::idles::warnings {
   class sibling_is_an_ancestor : public warning {
      public:
         constexpr sibling_is_an_ancestor(const idle_node& i) : idle(i) {}

      public:
         const idle_node& idle;
   };
}