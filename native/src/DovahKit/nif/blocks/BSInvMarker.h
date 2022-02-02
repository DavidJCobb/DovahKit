#pragma once
#include <array>
#include "NiExtraData.h"

namespace nifDK::block_types {
   class BSInvMarker : public NiExtraData {
      public:
         static constexpr const char* const type_name = "BSInvMarker";
      public:
         glm::fvec3 rotation = { 0, 0, 0 };
         float zoom = 1.0F;

         virtual void parse(file_reader&) override;
   };
}