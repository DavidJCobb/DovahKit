#pragma once
#include "NiObject.h"
#include "../types/extra_data_list.h"

namespace nifDK::block_types {
   class NiTimeController;

   class NiObjectNET : public NiObject {
      public:
         static constexpr const char* const type_name = "NiObjectNET";
      public:
         // <add name="Skyrim Shader Type" type="BSLightingShaderPropertyShaderType" vercond="User Version >= 12" cond="BSLightingShaderProperty">Configures the main shader path</add>
         std::string       name; // uint32_t length; chars;
         extra_data_list   extra;
         NiTimeController* controller = nullptr;

         virtual void parse(file_reader&) override;
   };
}