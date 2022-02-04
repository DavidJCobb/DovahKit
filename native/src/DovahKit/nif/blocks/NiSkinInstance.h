#pragma once
#include "NiObject.h"

namespace nifDK::block_types {
   class NiNode;
   class NiSkinData;
   class NiSkinPartition;

   class NiSkinInstance : public NiObject {
      public:
         static constexpr const char* const type_name = "NiSkinInstance";
      public:
         NiSkinData*      data      = nullptr;
         NiSkinPartition* partition = nullptr;
         struct {
            NiNode* root = nullptr; // unowned
            std::vector<NiNode*> bones; // unowned
         } armature;

         virtual void parse(file_reader&) override;
   };
}