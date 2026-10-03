#pragma once
#include <cstdint>
#include "lu/integral_character_code.h"
#include "./group_type.h"

namespace dovah::tes {
   struct group_header {
      lu::four_cc signature = 0; // should always be 'GRUP'
      uint32_t    size  = 0;
      uint32_t    label = 0;
      group_type  type  = (group_type)-1;
      union {
         struct {
            uint8_t vc_day;
            uint8_t vc_month;
            uint8_t vc_last_editor;
            uint8_t vc_current_editor;
         };
         uint32_t version_control = 0;
      };
      uint32_t unknown = 0;
   };
   static_assert(sizeof(group_header) == 0x18);
}
