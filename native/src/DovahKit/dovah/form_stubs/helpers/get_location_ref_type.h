#pragma once
#include "./get_unique_outbound_use.h"
#include "../../use_info/entry_flags/base_extra_data.h"

namespace dovah::form_stub_helpers {
   extern form_stub* get_location_ref_type(const form_stub& form) {
      return get_unique_outbound_use<use_info::entry_flags::base_extra_data::extra_location_ref_type>(form);
   }
}