#pragma once
#include "BSBlastNode.h"

namespace nifDK::block_types {
   class BSDamageStage : public BSBlastNode {
      public:
         static constexpr const char* const type_name = "BSDamageStage";
   };
}