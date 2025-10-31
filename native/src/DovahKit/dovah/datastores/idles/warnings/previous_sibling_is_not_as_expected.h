#pragma once
#include "./_base.h"
namespace dovah::datastores::impl::idles {
   class idle_node;
}

namespace dovah::datastores::impl::idles::warnings {
   class previous_sibling_is_not_as_expected : public warning {
      public:
         constexpr previous_sibling_is_not_as_expected(
            const idle_node& i,
            const idle_node* expected,
            const idle_node* actual
         ) :
            idle(i),
            previous_sibling({
               .expected = expected,
               .actual   = actual,
            })
         {}

      public:
         const idle_node& idle;
         struct {
            const idle_node* expected = nullptr;
            const idle_node* actual   = nullptr;
         } previous_sibling;
   };
}