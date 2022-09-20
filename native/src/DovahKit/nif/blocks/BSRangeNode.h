#pragma once
#include "NiNode.h"

namespace nifDK::block_types {
   class BSRangeNode : public NiNode {
      public:
         static constexpr const char* const type_name = "BSRangeNode";
      public:
         struct {
            uint8_t min = 0;
            uint8_t max = 255;
            uint8_t current = 255;
         } range;

         virtual void parse(file_reader&) override;
   };
}