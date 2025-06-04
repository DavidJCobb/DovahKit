#pragma once
#include "../../../base_form_load_warning.h"

#include "dovah/forms/IdleAnimation.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::idle {
   class filename_too_long : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr filename_too_long(
            form_stub& stub,
            size_t     size
         )
         :
            base_form_load_warning(stub),
            size(size)
         {}

         size_t size;
         size_t max_serializable_size = dovah::loaded_forms::IdleAnimation::max_filename_length;
   };
}
#include "../../../_util.undef.h"