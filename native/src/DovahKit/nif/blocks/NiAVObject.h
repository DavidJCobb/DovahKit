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
         struct flag {
            enum type : uint32_t {
               culled_by_application = 0x00000001,

               no_decals = 0x00000400,
            };
         };
      public:
         uint32_t flags = 0;
         NiTransform transform;
         NiCollisionObject* collision = nullptr;
         //
         NiNode* parent = nullptr; // set during load; not specified in the file

         virtual void parse(file_reader&) override;
   };
}