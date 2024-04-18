#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::quest {
   //
   // A Papyrus fragment is tagged as belonging to a log entry that isn't 
   // actually defined on this quest.
   //
   class papyrus_fragment_belongs_to_missing_log_entry : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr papyrus_fragment_belongs_to_missing_log_entry(
            form_stub& subject,
            uint16_t   stage_id,
            uint32_t   entry_index
         )
         :
            base_form_load_warning(subject),
            stage_id(stage_id),
            entry_index(entry_index)
         {}

         uint16_t stage_id;
         uint32_t entry_index;
   };
}
#include "../../../_util.undef.h"