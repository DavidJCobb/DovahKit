#pragma once
#include "../extra_data.h"

namespace dovah::loaded_forms::components::extra {
   class global : public formID_extra_data<'XSCL', extra_data_type::global> {
      // The form should be a GLOB.
   };
}