#pragma once
#include <glm/glm.hpp>
#include "bhkShapeCollection.h"
#include "../types/HavokFilter.h"
#include "../types/HavokMaterial.h"
#include "./NiTriStripsData.h"

namespace nifDK::block_types {
   class bhkNiTriStripsShape : public bhkShapeCollection {
      public:
         static constexpr const char* const type_name = "bhkNiTriStripsShape";
      public:
         struct strip {
            NiTriStripsData* data = nullptr;
            HavokFilter      filter;
         };

         HavokMaterial material;
         float radius;
         uint32_t pad08[2];
         uint32_t max_size; // unused
         uint32_t size;     // unused
         uint32_t e_size;   // unused
         uint32_t grow_by = 1;
         glm::fvec4 scale = { 1.0, 1.0, 1.0, 0.0 };
         std::vector<strip> strips;

         virtual void parse(file_reader&) override;
   };
}