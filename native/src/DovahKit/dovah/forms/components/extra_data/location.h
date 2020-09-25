#pragma once
#include "../extra_data.h"

namespace dovah::loaded_forms::components::extra {
   class location : public formID_extra_data<'XLCN', extra_data_type::location> {
      // The form should be a LCTN.
   };
}