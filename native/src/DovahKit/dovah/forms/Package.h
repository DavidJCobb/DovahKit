#pragma once
#include <cstdint>
#include <string>
#include "Form.h"
#include "_common.h"

#include "../../../incomplete_code_warnings.h"
static_assert(incomplete_code_warnings::allow_compiling_despite_incomplete_forms, "The backend for Package is incomplete.");

namespace dovah::loaded_forms {
   class Package : public Form {
      #include "impl/form_subclass_components.txt"
      //
      // Intentionally minimal for now.
      //
      public:
         static constexpr form_type_t form_type = form_type::package;
         Package(const constructor_params& c) : Form(form_type, c) {};
   };
}