#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_component::attack_data {
   //
   // The game assumes that the subrecord after ATKD is ATKE. Bethesda tried to check the signature, but 
   // made a mistake: they retrieve the signature, but they never actually do check it, and the caller(s) 
   // are arranged such that they've already opened the next subrecord anyway and would therefore skip it 
   // even if they did detect a mismatch.
   //
   class expected_event_subrecord : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr expected_event_subrecord(
            form_stub& subject,
            uint32_t   subrecord_signature
         )
         :
            base_form_load_warning(subject),
            subrecord_signature(subrecord_signature)
         {}

         uint32_t subrecord_signature = 0; // subrecord that appears where ATKE was expected
   };
}
#include "../../../_util.undef.h"