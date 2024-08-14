#include "./get_base_form.h"
#include "../../form_stub.h"
#include "../../form_types.h"
#include "./get_unique_outbound_use.h"

namespace dovah::form_stub_helpers {
   extern form_stub* get_base_form(const form_stub* ref) {
      if (!form_type_is_reference(ref->form_type))
         return nullptr;
      return get_unique_outbound_use<use_info_entry::flag::object_reference>(*ref);
   }
}