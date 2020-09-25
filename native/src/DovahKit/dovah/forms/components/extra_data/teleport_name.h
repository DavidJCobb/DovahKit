#pragma once
#include "../extra_data.h"

namespace dovah::loaded_forms::components::extra {
   class teleport_name : public formID_extra_data<'XTNM', extra_data_type::teleport_name> {
      // The form should be a MESG.
   };
}