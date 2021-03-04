#pragma once
#include <cstdint>
#include <string>
#include "Form.h"
#include "_common.h"

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