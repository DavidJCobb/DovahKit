#pragma once
#include "NiObjectNET.h"
#include "../types/NiTransform.h"

namespace nifDK::block_types {
   class NiCollisionObject;

   class NiAVObject : public NiObjectNET {
      public:
         static constexpr const char* const type_name = "NiAVObject";
      public:
         uint32_t flags = 0;
         // <add name="Unknown Short 1" type="ushort" default="8" ver1="20.2.0.7" vercond="(User Version >= 11) &amp;&amp; (User Version 2 > 26)" >Unknown Flag</add>
         NiTransform transform;
         NiCollisionObject* collision = nullptr;

         virtual void parse(file_reader&) override;
   };
}