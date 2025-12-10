#pragma once
#include "../warning.h"
namespace dovah::datastores::impl::camera_paths {
   class node;
}

namespace dovah::datastores::impl::camera_paths::warnings {
   class form_has_multiple_next_siblings : public warning {
      public:
         form_has_multiple_next_siblings(node& s) : subject(s) {};

      public:
         node& subject;
   };
}
