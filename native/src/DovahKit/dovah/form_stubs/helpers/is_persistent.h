#pragma once
#include "../../form_stub.h"
#include "../../form_types.h"

namespace dovah::form_stub_helpers {
   constexpr bool is_persistent(const form_stub* stub) {
      if (!stub)
         return false;
      if (stub->form_type == form_type::cell || form_type_is_reference(stub->form_type)) {
         return stub->test_record_flags(0x400);
      }
      return false;
   }
}