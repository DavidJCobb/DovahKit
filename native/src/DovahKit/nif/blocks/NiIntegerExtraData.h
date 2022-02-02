#pragma once
#include "NiExtraData.h"

namespace nifDK::block_types {
   class NiIntegerExtraData : public NiExtraData {
      public:
         static constexpr const char* const type_name = "NiIntegerExtraData";
      public:
         int32_t value = 0;

         virtual void parse(file_reader&) override;
   };
}