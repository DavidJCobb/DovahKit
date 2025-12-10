#pragma once
#include "../warning.h"
namespace dovah::datastores::impl::camera_paths {
   class node;
}

namespace dovah::datastores::impl::camera_paths::warnings {
   class inconsistent_parentage : public warning {
      public:
         inconsistent_parentage(node& s) : subject(s) {};

      public:
         node& subject;
   };
}