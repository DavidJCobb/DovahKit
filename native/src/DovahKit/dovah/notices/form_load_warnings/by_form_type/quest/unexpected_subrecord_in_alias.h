#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::quest {
   class unexpected_subrecord_in_alias final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr unexpected_subrecord_in_alias(
            form_stub& subject,
            uint32_t   alias_id,
            uint32_t   signature
         ) :
            base_form_load_warning(subject),
            alias_id(alias_id),
            signature(signature)
         {}

         uint32_t alias_id  = 0;
         uint32_t signature = 0;
   };
}
#include "../../../_util.undef.h"