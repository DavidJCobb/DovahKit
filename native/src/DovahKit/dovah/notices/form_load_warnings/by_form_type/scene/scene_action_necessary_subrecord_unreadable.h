#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::scene {
   //
   // When the scene loader encounters ANAM with a valid value, it feeds all 
   // subsequent subrecords to the to-be-loaded scene action until it either 
   // finishes reading the action or encounters an irrecoverable problem.
   // 
   // All scene action types have a specific subrecord that they consider 
   // absolutely necessary. If the subrecord is present but fails to read 
   // (e.g. because it's too small), then the game considers the entire 
   // action invalid: it stops reading the action (such that all subsequent 
   // subrecords go to the scene) and discards it.
   //
   class scene_action_necessary_subrecord_unreadable : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr scene_action_necessary_subrecord_unreadable(
            form_stub& stub,
            uint32_t   action_id,
            uint32_t   subrecord_signature
         )
         :
            base_form_load_warning(stub),
            action_id(action_id),
            subrecord_signature(subrecord_signature)
         {}

         uint32_t action_id;
         uint32_t subrecord_signature;
   };
}
#include "../../../_util.undef.h"