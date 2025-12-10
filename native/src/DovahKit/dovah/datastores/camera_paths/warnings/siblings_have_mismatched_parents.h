#pragma once
#include "../warning.h"
namespace dovah::datastores::impl::camera_paths {
   class node;
}

namespace dovah::datastores::impl::camera_paths::warnings {
   class siblings_have_mismatched_parents : public warning {
      public:
         siblings_have_mismatched_parents(node& s) : subject(s) {};

      public:
         node& subject;
   };
}