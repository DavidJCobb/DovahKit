#pragma once
#include "../enums/input_device_type.h"

struct DKVulkanCameraUpdate;
namespace dovahkit::subsystems::worldinput {
   class combined_tool_results;
   namespace binds {
      class node;
      namespace nodes {
         class root;
      }
   }
}

namespace dovahkit::subsystems::worldinput::binds {
   class tree {
      public:
         tree(input_device_type);
         //
         tree(const tree&);
         tree& operator=(const tree& o);
         //
         tree(tree&&);
         tree& operator=(tree&& o);

         input_device_type device;
         nodes::root*      root   = nullptr;
         node*             active = nullptr;

         void process(combined_tool_results& instant, combined_tool_results& while_down);
   };
}