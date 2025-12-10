#pragma once
#include "../warning.h"
namespace dovah::datastores::impl::camera_paths {
   class node;
}

namespace dovah::datastores::impl::camera_paths::warnings {
   class multiple_children_want_to_be_first : public warning {
      public:
         multiple_children_want_to_be_first(node& s) : subject(s) {};

      public:
         node& subject;
   };
}
