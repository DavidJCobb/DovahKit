#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::scene {
   class scene_actor_subrecords_out_of_order : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr scene_actor_subrecords_out_of_order(
            form_stub& stub,
            uint32_t   subrecord_signature
         )
         :
            base_form_load_warning(stub),
            subrecord_signature(subrecord_signature)
         {}

         uint32_t subrecord_signature;
   };
}
#include "../../../_util.undef.h"