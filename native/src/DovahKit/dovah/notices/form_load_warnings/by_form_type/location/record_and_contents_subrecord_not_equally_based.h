#pragma once
#include "./base_contents_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::location {
   // e.g. ACPR in a base record, or LCPR in an override
   class record_and_contents_subrecord_not_equally_based : public base_contents_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr record_and_contents_subrecord_not_equally_based(
            form_stub& stub,
            contents_type type,
            bool is_base_record
         )
         :
            base_contents_warning(stub),
            type(type),
            is_base_record(is_base_record)
         {}

         contents_type type;
         bool is_base_record;
   };
}
#include "../../../_util.undef.h"