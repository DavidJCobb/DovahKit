#pragma once
#include "../warning.h"
namespace dovah::datastores::impl::camera_paths {
   class node;
}

namespace dovah::datastores::impl::camera_paths::warnings {
   class sibling_is_an_ancestor : public warning {
      public:
         sibling_is_an_ancestor(node& s) : subject(s) {};

      public:
         node& subject;
   };
}