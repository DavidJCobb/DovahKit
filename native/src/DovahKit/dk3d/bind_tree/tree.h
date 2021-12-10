#pragma once
#include "../enums/InputDevice.h"

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
         tree(InputDevice);
         //
         tree(const tree&);
         tree& operator=(const tree& o);
         //
         tree(tree&&);
         tree& operator=(tree&& o);

         InputDevice  device;
         nodes::root* root   = nullptr;
         node*        active = nullptr;

         void process(DKVulkanCameraUpdate&);
   };
}