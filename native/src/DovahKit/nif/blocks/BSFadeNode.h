#pragma once
#include "NiNode.h"

namespace nifDK::block_types {
   class BSFadeNode : public NiNode {
      public:
         static constexpr const char* const type_name = "BSFadeNode";
      public:
         //
         // at run-time, this class has fields for fading objects in and out as part of LOD and spawn 
         // effects; in files, however, it's basically just a NiNode
         //
   };
}