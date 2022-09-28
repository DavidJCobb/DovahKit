#pragma once
#include <glm/glm.hpp>
#include "bhkShapeCollection.h"
#include "../types/HavokMaterial.h"
#include "../types/hkWorldObjCinfoProperty.h"

namespace nifDK::block_types {
   class bhkListShape : public bhkShapeCollection {
      public:
         static constexpr const char* const type_name = "bhkListShape";
      public:
         std::vector<bhkShape*> contents;
         HavokMaterial material;
         hkWorldObjCinfoProperty child_shape_property;
         hkWorldObjCinfoProperty child_filter_property;
         std::vector<uint32_t> unknown;

         virtual void parse(file_reader&) override;
   };
}