#pragma once
#include "../../../base_form_load_warning.h"

#include "dovah/forms/Race.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::race {
   class biped_object_name_too_long : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr biped_object_name_too_long(
            form_stub& stub,
            size_t     size,
            size_t     which
         )
         :
            base_form_load_warning(stub),
            size(size),
            which(which)
         {}

         size_t size;
         size_t max_serializable_size = dovah::loaded_forms::Race::max_biped_object_name_length;
         size_t which;
   };
}
#include "../../../_util.undef.h"