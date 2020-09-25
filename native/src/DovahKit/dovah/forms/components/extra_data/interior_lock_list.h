#pragma once
#include "../extra_data.h"

namespace dovah::loaded_forms::components::extra {
   class interior_lock_list : public formID_extra_data<'XILL', extra_data_type::interior_lock_list> {
      // The form should be an FLST or NPC_.
   };
}