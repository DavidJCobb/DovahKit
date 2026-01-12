#include "./get_base_form.h"
#include "./get_unique_outbound_use.h"
#include "../../use_info/entry_flags/reference.h"

namespace dovah::form_stub_helpers {
   extern form_stub* get_base_form(const form_stub& ref) {
      return get_unique_outbound_use<use_info::entry_flags::reference::base_form>(ref);
   }
}