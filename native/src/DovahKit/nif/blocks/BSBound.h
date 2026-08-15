#pragma once
#include "NiExtraData.h"

namespace nifDK::block_types {
   class BSBound : public NiExtraData {
      public:
         static constexpr const char* const type_name = "BSBound";
      public:
         glm::fvec3 center     = { 0, 0, 0 };
         glm::fvec3 halfwidths = { 0, 0, 0 };

         virtual void parse(file_reader&) override;
   };
}