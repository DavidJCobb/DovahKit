#pragma once
#include "NiObject.h"

namespace nifDK::block_types {
   class NiAVObject;

   class NiCollisionObject : public NiObject {
      public:
         static constexpr const char* const type_name = "NiCollisionObject";
      public:
         NiAVObject* owner = nullptr; // unowned

         virtual void parse(file_reader&) override;
   };
}