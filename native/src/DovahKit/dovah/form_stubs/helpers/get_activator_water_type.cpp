#include "./get_activator_water_type.h"
#include "../../form_stub.h"
#include "../../form_types.h"
#include "./get_unique_outbound_use.h"

namespace dovah::form_stub_helpers {
   extern form_stub* get_activator_water_type(const form_stub& activator) {
      if (activator.form_type != form_type::activator)
         return nullptr;
      return get_unique_outbound_use<use_info_entry::flag::water_acti_type>(activator);
   }
}