#pragma once
#include "../extra_data.h"

namespace dovah::loaded_forms::components::extra {
   class leveled_item_base : public formID_extra_data<'LVLI', extra_data_type::leveled_item_base> {
      // The form should be an LVLI.
   };
}