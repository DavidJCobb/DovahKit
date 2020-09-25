#pragma once
#include "../extra_data.h"

namespace dovah::loaded_forms::components::extra {
   class location_ref_type : public formID_extra_data<'XLRT', extra_data_type::location_ref_type> {
      // The form should be an LCRT.
   };
}