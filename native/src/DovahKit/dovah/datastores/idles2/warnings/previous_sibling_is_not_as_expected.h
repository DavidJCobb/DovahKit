#pragma once
#include "../warning.h"
namespace dovah::datastores::impl::idles2 {
   class idle_node;
}

namespace dovah::datastores::impl::idles2::warnings {
   class previous_sibling_is_not_as_expected : public warning {
      public:
         previous_sibling_is_not_as_expected(
            idle_node& s,
            idle_node* i,
            idle_node* a
         ) :
            subject(s),
            sibling({ i, a })
         {};

      public:
         idle_node& subject;
         struct {
            idle_node* intended = nullptr;
            idle_node* actual   = nullptr;
         } sibling;
   };
}