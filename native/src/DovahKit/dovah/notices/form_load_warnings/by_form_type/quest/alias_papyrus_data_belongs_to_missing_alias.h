#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::quest {
   //
   // A quest's VMAD subrecord contained Papyrus data for an alias ID that isn't 
   // defined on the quest.
   //
   class alias_papyrus_data_belongs_to_missing_alias : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr alias_papyrus_data_belongs_to_missing_alias(
            form_stub& subject,
            uint16_t   alias
         )
         :
            base_form_load_warning(subject),
            alias_id(alias)
         {}

         uint16_t   alias_id;
   };
}
#include "../../../_util.undef.h"