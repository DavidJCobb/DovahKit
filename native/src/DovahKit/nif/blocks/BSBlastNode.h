#pragma once
#include "BSRangeNode.h"

namespace nifDK::block_types {
   class BSBlastNode : public BSRangeNode {
      public:
         static constexpr const char* const type_name = "BSBlastNode";
   };
}