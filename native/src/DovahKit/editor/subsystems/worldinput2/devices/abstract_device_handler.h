#pragma once
#include <QPointF>
#include "../enums/axis2D.h"
#include "../enums/button_press_type.h"
#include "../enums/range_control_state.h"
#include "../enums/scalar_input_control.h"
#include "../enums/vector_input_control.h"
#include "../chrono.h"
#include "../device_button_state.h"
#include "../interruption_check.h"

namespace dovahkit::subsystems::worldinput2 {
   namespace inputs {
      struct button;
   }
   struct device_button_claim;
}

namespace dovahkit::subsystems::worldinput2::devices {
   class abstract_device_handler {
      public:
         virtual ~abstract_device_handler() {}

         virtual device_button_state get_state_of(const inputs::button&) const = 0;
         virtual const device_button_claim& get_existing_claim_of(const inputs::button&) const = 0;
         virtual device_button_claim& get_pending_claim_of(const inputs::button&) = 0;
         virtual const device_button_claim& get_pending_claim_of(const inputs::button&) const = 0;

         virtual bool button_is_valid(const inputs::button&) const = 0;

         virtual range_control_state get_range_control_state(scalar_input_control, axis2D) const = 0;
         virtual range_control_state get_range_control_state(vector_input_control) const = 0;
         virtual QPointF get_range_control_value(scalar_input_control, axis2D) const = 0;
         virtual QPointF get_range_control_value(vector_input_control) const = 0;

         interruption_check prepare_interruption_check() const;

      protected:
         virtual interruption_check _prepare_interruption_check_impl() const = 0;
   };
}