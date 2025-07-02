#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::package {
   class unique_id_has_multiple_metadata final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

      public:
         constexpr unique_id_has_multiple_metadata(
            form_stub& subject,
            uint8_t unique_id
         )
         :
            base_form_load_warning(subject),
            unique_id(unique_id)
         {}

         uint8_t unique_id;
   };
}
#include "../../../_util.undef.h"