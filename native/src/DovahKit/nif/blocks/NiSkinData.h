#pragma once
#include "NiObject.h"
#include "../types/NiBound.h"
#include "../types/NiTransform.h"

namespace nifDK::block_types {
   class NiSkinPartition;

   class NiSkinData : public NiObject {
      public:
         static constexpr const char* const type_name = "NiSkinData";
      public:
         struct bone {
            struct vertex {
               static constexpr size_t serialized_size      = 6;
               static constexpr size_t serialized_size_half = 4;

               uint16_t index;
               float    weight = 0.0; // [0, 1]
               
               void unchecked_read(file_reader&);
            };

            NiTransform transform;
            NiBound     bound;
            std::vector<vertex> vertex_weights;

            void parse(file_reader&, uint8_t presence);
         };

         NiTransform      transform;
         NiSkinPartition* partition = nullptr; // old NIFs only; modern NIFs specify it on the skin instance
         std::vector<bone> bones;

         virtual void parse(file_reader&) override;
   };
}