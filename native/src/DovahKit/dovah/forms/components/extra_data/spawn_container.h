#pragma once
#include "../extra_data.h"

namespace dovah::loaded_forms::components::extra {
   class spawn_container : public formID_extra_data<'XSPC', extra_data_type::spawn_container> {
      // The form should be a REFR.
   };
}