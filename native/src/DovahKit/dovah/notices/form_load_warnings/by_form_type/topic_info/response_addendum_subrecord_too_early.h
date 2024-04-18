#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::topic_info {
   //
   // The TRDT subrecord begins a response, and subrecords like NAM1 set specific fields 
   // on that response. This warning is emitted if one of those addenda subrecords is seen 
   // before the first TRDT.
   //
   class response_addendum_subrecord_too_early : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr response_addendum_subrecord_too_early(
            form_stub& subject,
            uint32_t   subrecord_signature
         )
         :
            base_form_load_warning(subject),
            subrecord_signature(subrecord_signature)
         {}

         uint32_t subrecord_signature = 0;
   };
}
#include "../../../_util.undef.h"