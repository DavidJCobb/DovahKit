#pragma once
#include "NiObjectNET.h"
#include "../types/NiTransform.h"

namespace nifDK::block_types {
   class NiCollisionObject;
   class NiNode;

   class NiAVObject : public NiObjectNET {
      public:
         static constexpr const char* const type_name = "NiAVObject";
      public:
         uint32_t flags = 0;
         NiTransform transform;
         NiCollisionObject* collision = nullptr;
         //
         NiNode* parent = nullptr; // set during load; not specified in the file

         virtual void parse(file_reader&) override;
   };
}