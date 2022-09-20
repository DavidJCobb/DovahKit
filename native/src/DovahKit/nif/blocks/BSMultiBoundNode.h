#pragma once
#include "NiNode.h"

namespace nifDK::block_types {
   class BSMultiBound;

   class BSMultiBoundNode : public NiNode {
      public:
         static constexpr const char* const type_name = "BSMultiBoundNode";
      public:
         enum class culling_modes : uint32_t {
            normal = 0,
            all_pass = 1,
            all_fail = 2,
            ignore_multibounds = 3,
            force_multibounds_no_update = 4,
         };
      public:
         BSMultiBound* multibound   = nullptr; // unowned
         culling_modes culling_mode = culling_modes::normal; // user version 2 >= 83

         virtual void parse(file_reader&) override;
   };
}