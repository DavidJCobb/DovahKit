#pragma once
#include <cstdint>
#include <string>
#include "Form.h"
#include "_common.h"

namespace dovah::loaded_forms {
   class Package : public Form, _IncompleteFormType {
      //
      // Intentionally minimal for now.
      //
      public:
         static constexpr form_type_t form_type = form_type::package;
         Package(const constructor_params& c) : Form(form_type, c) {};
   };
}