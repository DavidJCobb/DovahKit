#pragma once
#include "../devices/abstract_device_handler.h"
#include "../chrono.h"

namespace dovahkit::subsystems::worldinput {
   class bind_list_item;
}

namespace dovahkit::subsystems::worldinput::algorithms {
   enum class press_preempt_hold_result {
      no_conflict,
      press_delays_hold,
      press_blocks_hold,
      press_advanced_past_hold,
      hold_outlasted_press,
   };

   extern press_preempt_hold_result press_preempts_hold(
      timestamp_t current_time,
      devices::abstract_device_handler&,
      const bind_list_item& press,
      const bind_list_item& hold
   );
}