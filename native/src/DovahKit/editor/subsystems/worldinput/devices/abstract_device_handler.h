#pragma once
#include <QPointF>
#include "../enums/axis2D.h"
#include "../enums/button_press_type.h"
#include "../enums/range_control_state.h"
#include "../enums/range_input_axes.h"
#include "../enums/range_input_control.h"
#include "../chrono.h"
#include "../device_button_state.h"
#include "../interruption_check.h"
#include "../raycast_result.h"

namespace dovah {
   class form_stub;
}
namespace dovahkit::subsystems::worldinput {
   namespace inputs {
      struct button;
   }
   struct device_button_claim;
}

namespace dovahkit::subsystems::worldinput::devices {
   class abstract_device_handler {
      public:
         virtual ~abstract_device_handler() {}

         virtual device_button_state get_state_of(const inputs::button&) const = 0;
         virtual const device_button_claim& get_existing_claim_of(const inputs::button&) const = 0;
         virtual device_button_claim& get_pending_claim_of(const inputs::button&) = 0;
         virtual const device_button_claim& get_pending_claim_of(const inputs::button&) const = 0;

         virtual bool button_is_valid(const inputs::button&) const = 0;

         virtual range_control_state get_range_control_state(range_input_control, range_input_axes) const = 0;
         virtual QPointF get_range_control_value(range_input_control, range_input_axes) const = 0; // If only one axis is requested, return { axis_value, 0 }.

         // The return value is stored in a `std::vector` and is not heap-allocated.
         //
         // - It IS NOT SAFE to keep a reference to the return value for longer than 
         //   the current stack frame; consider it a temporary.
         //
         // - If you're calling this function multiple times (which you generally 
         //   shouldn't need to do given how Worldinput is meant to handle raycasts), 
         //   then DO NOT store references to the return values. Each call to this 
         //   function may potentially create a new raycast result, invalidating all 
         //   iterators (and consequently references) for older raycasts. If you need 
         //   to make multiple calls from the same place, copy the values instead of 
         //   using reference-typed variables.
         //
         const raycast_result_per_key& get_raycast_result(timestamp_t now, const inputs::button&);

         const raycast_result& get_per_frame_raycast_result();

         void discard_raycast_results_for(const dovah::form_stub&);

         interruption_check prepare_interruption_check() const;

      protected:
         virtual interruption_check _prepare_interruption_check_impl() const = 0;

      protected:
         QPoint pointer_position; // view-relative and in view coordinates i.e. pixels
         struct {
            std::vector<raycast_result_per_key> per_button;
            std::optional<raycast_result>       this_frame;
         } raycast_results;
   };
}