#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::reference {
   class suspiciously_low_z_position : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr suspiciously_low_z_position(
            form_stub& stub,
            float sus_threshold,
            float actual
         )
         :
            base_form_load_warning(stub),
            sus_threshold(sus_threshold),
            actual(actual)
         {}

         float sus_threshold;
         float actual;
   };
}
#include "../../../_util.undef.h"