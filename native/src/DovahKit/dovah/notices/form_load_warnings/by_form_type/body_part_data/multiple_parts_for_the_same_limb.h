#pragma once
#include "../../../base_form_load_warning.h"
#include "../../../../data/limbs.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::body_part_data {
   class multiple_parts_for_the_same_limb : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr multiple_parts_for_the_same_limb(
            form_stub& stub,
            dovah::limb limb,
            size_t count
         )
         :
            base_form_load_warning(stub),
            limb(limb),
            count(count)
         {}

         size_t      count;
         dovah::limb limb;
   };
}
#include "../../../_util.undef.h"