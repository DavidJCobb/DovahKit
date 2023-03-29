#pragma once
#include <vector>
#include "../enums/input_device_type.h"

namespace dovahkit::subsystems::worldinput2 {
   class combined_tool_results;
   namespace binds {
      class node;
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

         // -----

         input_device_type device;
         nodes::root*      root   = nullptr;

         // If these binds cease to be active on this frame, then we must fire key-up invocations 
         // for them, so they can deactivate their effects as needed.
         std::vector<node*> last_frame_active_hold_binds;

         // -----

         void process(combined_tool_results& instant, combined_tool_results& while_down);
   };
}