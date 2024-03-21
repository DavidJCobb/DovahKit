#pragma once
#include "NiNode.h"

namespace nifDK::block_types {
   class BSValueNode : public NiNode {
      public:
         static constexpr const char* const type_name = "BSValueNode";
      public:
         struct flag {
            enum type : uint8_t {
               billboard_world_z = 1 << 0,
               use_player_adjust = 1 << 1,
            };
         };
         using flags_t = std::underlying_type_t<flag::type>;

         uint32_t value = 0;
         flags_t  flags = 0;

         virtual void parse(file_reader&) override;
   };
}