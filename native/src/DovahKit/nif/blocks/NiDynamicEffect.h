#pragma once
#include "NiAVObject.h"

namespace nifDK::block_types {
   class NiDynamicEffect : public NiAVObject {
      public:
         static constexpr const char* const type_name = "NiDynamicEffect";
      public:
         bool enabled = true;
         std::vector<NiNode*> targets; // not owned

         virtual void parse(file_reader&) override;
   };
}