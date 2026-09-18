#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::imagespace_modifier {
   class keyframe_position_out_of_range : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr keyframe_position_out_of_range(
            form_stub& subject,
            uint32_t   subrecord_signature,
            size_t     keyframe_index,
            float      keyframe_position
         )
         :
            base_form_load_warning(subject),
            subrecord_signature(subrecord_signature),
            keyframe({
               .index    = keyframe_index,
               .position = keyframe_position,
            })
         {}

         uint32_t subrecord_signature = 0;
         struct {
            size_t index;
            float  position;
         } keyframe;
   };
}
#include "../../../_util.undef.h"