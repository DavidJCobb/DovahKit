#pragma once
#include <array>
#include "NiNode.h"

namespace nifDK::block_types {
   class BSOrderedNode : public NiNode {
      public:
         static constexpr const char* const type_name = "BSOrderedNode";
      public:
         std::array<float, 4> alpha_sort_bound = {};
         bool static_bound = false;

         virtual void parse(file_reader&) override;
   };
}