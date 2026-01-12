#include "./get_activator_water_type.h"
#include "./get_unique_outbound_use.h"
#include "../../use_info/entry_flags/activator.h"
#include "../../use_info/entry_flags/furniture.h"

namespace dovah::form_stub_helpers {
   extern form_stub* get_activator_water_type(const form_stub& activator) {
      switch (activator.form_type) {
         case form_type::activator:
            return get_unique_outbound_use<use_info::entry_flags::activator::water_type>(activator);
         case form_type::furniture:
            return get_unique_outbound_use<use_info::entry_flags::furniture::water_type>(activator);
      }
      return nullptr;
   }
}