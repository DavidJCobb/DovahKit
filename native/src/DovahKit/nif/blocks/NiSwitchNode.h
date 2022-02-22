#pragma once
#include <type_traits>
#include "NiNode.h"

namespace nifDK::block_types {
   class NiSwitchNode : public NiNode {
      public:
         static constexpr const char* const type_name = "NiSwitchNode";
      public:
         struct flag {
            enum type : uint16_t {
               update_only_active_child = 0,
               update_controllers       = 1,
            };
         };
         using flags_t = std::underlying_type_t<flag::type>;

         flags_t  flags = 0;
         uint32_t current_child_index = 0;

         virtual void parse(file_reader&) override;

         NiAVObject* current_child() const;
   };
}