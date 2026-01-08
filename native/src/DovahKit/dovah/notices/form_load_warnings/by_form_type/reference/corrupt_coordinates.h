#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::reference {
   class corrupt_coordinates : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr corrupt_coordinates(
            form_stub& stub,
            bool       position,
            bool       rotation
         )
         :
            base_form_load_warning(stub),
            position(position),
            rotation(rotation)
         {}

         bool position = false;
         bool rotation = false;
   };
}
#include "../../../_util.undef.h"