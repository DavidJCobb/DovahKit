#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::cell {
   //
   // A subrecord specific to interior cells or exterior cells was seen before the DATA 
   // subrecord, which tells us whether the current cell is an interior or exterior cell.
   //
   class cell_type_not_yet_known : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr cell_type_not_yet_known(
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