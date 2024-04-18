#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::landscape {
   //
   // A landscape quad had too many texture layers. The limit is six, with any 
   // layers that specify higher indices overwriting the sixth layer instead.
   //
   class excess_layers_per_quad : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr excess_layers_per_quad(
            form_stub& subject,
            uint8_t    q,
            uint16_t   l,
            uint32_t   subrecord_signature,
            form_stub* texture
         )
         :
            base_form_load_warning(subject),
            quad(q),
            layer(l),
            subrecord_signature(subrecord_signature),
            texture(texture)
         {}

         uint32_t   subrecord_signature = 0;
         uint8_t    quad  = 0;
         uint16_t   layer = 0;
         form_stub* texture = nullptr;
   };
}
#include "../../../_util.undef.h"