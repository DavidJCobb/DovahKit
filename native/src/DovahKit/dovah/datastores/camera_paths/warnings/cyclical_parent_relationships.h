#pragma once
#include "../warning.h"
namespace dovah::datastores::impl::camera_paths {
   class node;
}

namespace dovah::datastores::impl::camera_paths::warnings {
   class cyclical_parent_relationships : public warning {
      public:
         cyclical_parent_relationships(node& s) : subject(s) {};

      public:
         node& subject;
   };
}