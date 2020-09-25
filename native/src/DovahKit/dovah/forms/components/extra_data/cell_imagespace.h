#pragma once
#include "../extra_data.h"

namespace dovah::loaded_forms::components::extra {
   class cell_imagespace : public formID_extra_data<'XCIM', extra_data_type::cell_imagespace> {
      // The form should be a IMGS.
   };
}