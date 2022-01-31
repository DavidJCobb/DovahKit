#pragma once
#include "NiAVObject.h"

namespace nifDK::block_types {
   class NiDynamicEffect;

   class NiNode : public NiAVObject {
      public:
         static constexpr const char* const type_name = "NiNode";
      public:
         std::vector<NiAVObject*>      children;
         std::vector<NiDynamicEffect*> effects;

         virtual void parse(file_reader&) override;
   };
}