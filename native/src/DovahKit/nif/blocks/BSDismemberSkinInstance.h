#pragma once
#include "NiSkinInstance.h"
#include "../types/BodyPartList.h"

namespace nifDK::block_types {
   class BSDismemberSkinInstance : public NiSkinInstance {
      public:
         static constexpr const char* const type_name = "BSDismemberSkinInstance";
      public:
         std::vector<BodyPartList> body_parts_per_partition;

         virtual void parse(file_reader&) override;
   };
}