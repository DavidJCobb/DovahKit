#include "./is_defined_or_overridden_in_active_file.h"
#include "dovah/active_load_order.h"
#include "dovah/form_stub.h"

namespace dovah::utils::form_stubs {
   extern bool is_defined_or_overridden_in_active_file(const form_stub& stub) {
      auto* active_file = stub.get_load_order().get_active_file();
      if (!active_file)
         return false;
      return stub.file_list_includes(*active_file);
   }
}