#pragma once
#include "../enums/input_device_type.h"

namespace dovahkit::subsystems::worldinput2 {
   namespace binds {
      namespace nodes {
         class root;
      }
   }
}

namespace dovahkit::subsystems::worldinput2::binds {
   class tree {
      public:
         tree(input_device_type);
         //
         tree(const tree&);
         tree& operator=(const tree& o);
         //
         tree(tree&&);
         tree& operator=(tree&& o);

         ~tree();

         // -----

         input_device_type device_type;
         nodes::root*      root   = nullptr;
   };
}