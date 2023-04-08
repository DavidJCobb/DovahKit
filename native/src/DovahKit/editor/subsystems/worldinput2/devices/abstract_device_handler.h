#pragma once
#include "../enums/button_press_type.h"
#include "../chrono.h"
#include "../device_button_state.h"

namespace dovahkit::subsystems::worldinput2 {
   class interruption_check;

   namespace inputs {
      struct button;
   }
}

namespace dovahkit::subsystems::worldinput2::devices {
   class abstract_device_handler {
      public:
         virtual ~abstract_device_handler();

         virtual device_button_state get_state_of(const inputs::button&) const = 0;
         virtual bool is_consumed(const inputs::button&) const = 0;
         virtual void consume(const inputs::button&) = 0;
         virtual void prepare_interruption_check(interruption_check&) const = 0;
   };
}