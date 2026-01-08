#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::reference {
   // See dovah::spaces::interior_cell_max_sane_bounds.
   class lateral_position_too_far_from_interior_origin : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr lateral_position_too_far_from_interior_origin(
            form_stub& stub,
            bool x_far,
            bool y_far
         )
         :
            base_form_load_warning(stub),
            x_far(x_far),
            y_far(y_far)
         {}

         bool x_far;
         bool y_far;
   };
}
#include "../../../_util.undef.h"