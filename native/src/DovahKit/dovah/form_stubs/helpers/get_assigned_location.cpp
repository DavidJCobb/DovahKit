#include "./get_assigned_location.h"
#include "./get_unique_outbound_use.h"
#include "../../use_info/entry_flags/base_extra_data.h"
#include "../../use_info/entry_flags/encounter_zone.h"
#include "../../use_info/entry_flags/worldspace.h"

namespace dovah::form_stub_helpers {
   extern form_stub* get_assigned_location(const form_stub& form) {
      if (form.form_type == dovah::form_type::encounter_zone) {
         return get_unique_outbound_use<use_info::entry_flags::encounter_zone::location>(form);
      } else if (form.form_type == dovah::form_type::worldspace) {
         return get_unique_outbound_use<use_info::entry_flags::worldspace::location>(form);
      } else {
         return get_unique_outbound_use<use_info::entry_flags::base_extra_data::extra_location>(form);
      }
   }
}