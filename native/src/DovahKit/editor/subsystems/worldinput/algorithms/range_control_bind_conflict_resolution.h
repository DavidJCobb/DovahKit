#pragma once
#include "../enums/range_input_axes.h"
#include "../util/range_control_conflict_state.h"

namespace dovahkit::subsystems::worldinput {
   class bind_list_item;
}

namespace dovahkit::subsystems::worldinput::algorithms {
   struct range_control_conflict_result {
      util::range_control_conflict_state a_outcome;
      util::range_control_conflict_state b_outcome;
   };

   extern range_control_conflict_result range_control_bind_conflict_resolution(
      const bind_list_item& a,
      const bind_list_item& b
   );
}