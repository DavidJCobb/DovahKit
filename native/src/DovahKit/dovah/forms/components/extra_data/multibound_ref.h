#pragma once
#include "../extra_data.h"

namespace dovah::loaded_forms::components::extra {
   class multibound_ref : public formID_extra_data<'XMBR', extra_data_type::multibound_ref> {
      // The form should be a REFR.
   };
}