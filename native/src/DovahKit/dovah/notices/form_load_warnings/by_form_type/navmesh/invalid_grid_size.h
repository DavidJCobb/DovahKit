#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::navmesh {
   class invalid_grid_size final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

      public:
         constexpr invalid_grid_size(
            form_stub& subject,
            size_t     size
         )
         :
            base_form_load_warning(subject),
            size(size)
         {}

         size_t size;
   };
}
#include "../../../_util.undef.h"