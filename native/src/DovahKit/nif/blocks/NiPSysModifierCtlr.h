#pragma once
#include "NiSingleInterpController.h"

namespace nifDK::block_types {
   class NiPSysModifierCtlr : public NiSingleInterpController {
      public:
         static constexpr const char* const type_name = "NiPSysModifierCtlr";
      public:
         std::string modifier_name; // name of a NiPSysModifier block in the same file

         virtual void parse(file_reader&) override;
   };
}