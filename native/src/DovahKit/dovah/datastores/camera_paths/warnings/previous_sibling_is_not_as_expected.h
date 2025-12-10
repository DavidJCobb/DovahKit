#pragma once
#include "../warning.h"
namespace dovah::datastores::impl::camera_paths {
   class node;
}

namespace dovah::datastores::impl::camera_paths::warnings {
   class previous_sibling_is_not_as_expected : public warning {
      public:
         previous_sibling_is_not_as_expected(
            node& s,
            node* i,
            node* a
         ) :
            subject(s),
            sibling({ i, a })
         {};

      public:
         node& subject;
         struct {
            node* intended = nullptr;
            node* actual   = nullptr;
         } sibling;
   };
}