#pragma once
#include "./_base.h"
namespace dovah::datastores::impl::idles {
   class idle_node;
}

namespace dovah::datastores::impl::idles::warnings {
   class siblings_have_mismatched_parents : public warning {
      public:
         constexpr siblings_have_mismatched_parents(const idle_node& i) : idle(i) {}

      public:
         const idle_node& idle;
   };
}