#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::weather {
   class extra_directional_ambient_lighting_params final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

      public:
         constexpr extra_directional_ambient_lighting_params(
            form_stub& stub,
            size_t     count_seen
         )
         :
            base_form_load_warning(stub),
            count_seen(count_seen)
         {}

         size_t count_seen;
         size_t max_count = 4;
   };
}
#include "../../../_util.undef.h"