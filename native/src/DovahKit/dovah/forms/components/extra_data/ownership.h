#pragma once
#include "../extra_data.h"

namespace dovah::loaded_forms::components::extra {
   class ownership : public formID_extra_data<'XOWN', extra_data_type::ownership> {
      // The form should be a FACT or NPC_.
   };
}