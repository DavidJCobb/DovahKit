#include "./get_assigned_encounter_zone.h"
#include "./get_unique_outbound_use.h"
#include "../../use_info/entry_flags/base_extra_data.h"
#include "../../use_info/entry_flags/worldspace.h"

namespace dovah::form_stub_helpers {
   extern form_stub* get_assigned_encounter_zone(const form_stub& form) {
      if (form.form_type == dovah::form_type::worldspace) {
         return get_unique_outbound_use<use_info::entry_flags::worldspace::encounter_zone>(form);
      } else {
         return get_unique_outbound_use<use_info::entry_flags::base_extra_data::extra_encounter_zone>(form);
      }
   }
}