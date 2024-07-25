#pragma once
#include "../../../base_form_load_warning.h"

#include "dovah/forms/Race.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::race {
   class too_many_biped_object_names : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr too_many_biped_object_names(
            form_stub& stub,
            size_t     count
         )
         :
            base_form_load_warning(stub),
            count(count)
         {}

         size_t count;
         size_t max_count = dovah::loaded_forms::Race::max_biped_object_name_count;
   };
}
#include "../../../_util.undef.h"