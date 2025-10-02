#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_component::keyword_list {
   //
   // The game assumes that the subrecord after KSIZ is KWDA.
   //
   class expected_array_subrecord : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr expected_array_subrecord(
            form_stub& subject,
            uint32_t   subrecord_signature
         )
         :
            base_form_load_warning(subject),
            subrecord_signature(subrecord_signature)
         {}

         uint32_t subrecord_signature = 0; // subrecord that appears where KWDA was expected
   };
}
#include "../../../_util.undef.h"