#pragma once

namespace dovahkit::subsystems::worldinput {
   class bind_list_item;
}

namespace dovahkit::subsystems::worldinput::algorithms {
   extern bool hold_blocks_press(
      const bind_list_item& press,
      const bind_list_item& hold
   );
}