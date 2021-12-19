#pragma once
#include "../enums/input_device_type.h"

struct DKVulkanCameraUpdate;
namespace DK3D::binds {
   class node;
   namespace nodes {
      class root;
   }
}

namespace DK3D::binds {
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

         void process(DKVulkanCameraUpdate&);
   };
}