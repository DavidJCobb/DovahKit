#pragma once
#include "NiObject.h"

namespace nifDK::block_types {
   class NiExtraData;
   class NiTimeController;

   class NiObjectNET : public NiObject { // "NiObject with name, extra data, and time controller"
      public:
         static constexpr const char* const type_name = "NiObjectNET";
      public:
         std::string       name; // uint32_t length; chars;
         std::vector<NiExtraData*> extra;
         NiTimeController* controller = nullptr;

         virtual void parse(file_reader&) override;
   };
}