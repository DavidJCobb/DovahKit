#include "./can_load_form_data.h"
#include "dovah/form_data/can_load_form_data_of_type.h"
#include "dovah/form_stub.h"
#include "dovah/form_type.h"

namespace dovah::utils::form_stubs {
   extern bool can_load_form_data(const form_stub& stub) {
      if (!can_load_form_data_of_type(stub.form_type))
         return false;
      if (!stub.has_source_files())
         return false;
      if (stub.is_none_stub())
         return false;
      return true;
   }
}