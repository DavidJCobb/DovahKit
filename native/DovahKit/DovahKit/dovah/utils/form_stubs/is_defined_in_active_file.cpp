#include "./is_defined_in_active_file.h"
#include "dovah/active_load_order.h"
#include "dovah/form_stub.h"

namespace dovah::utils::form_stubs {
   extern bool is_defined_in_active_file(const form_stub& stub) {
      return stub.get_file_at_index(0) == stub.get_load_order().get_active_file();
   }
}