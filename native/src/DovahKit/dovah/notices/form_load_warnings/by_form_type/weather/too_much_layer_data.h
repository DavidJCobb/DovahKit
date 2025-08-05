#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::weather {
   class too_much_layer_data final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         enum class data_type {
            alpha,
            color,
         };

      public:
         constexpr too_much_layer_data(
            form_stub& stub,
            data_type  type,
            size_t     count_seen
         )
         :
            base_form_load_warning(stub),
            type(type),
            count_seen(count_seen)
         {}

         data_type type;
         size_t count_seen;
         size_t max_count = 32;
   };
}
#include "../../../_util.undef.h"