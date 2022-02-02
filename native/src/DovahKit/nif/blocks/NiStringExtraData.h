#pragma once
#include "NiExtraData.h"

namespace nifDK::block_types {
   class NiStringExtraData : public NiExtraData {
      public:
         static constexpr const char* const type_name = "NiStringExtraData";
      public:
         std::string value;

         virtual void parse(file_reader&) override;
   };
}