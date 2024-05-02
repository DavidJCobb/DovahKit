#pragma once
#include <cstdint>
#include <optional>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_component::extra_data {
   class room_ref_data_insufficient_rooms : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr room_ref_data_insufficient_rooms(
            form_stub& subject,
            size_t expected,
            size_t found
         )
         :
            base_form_load_warning(subject),
            expected(expected),
            found(found)
         {}
         
         size_t expected = 0;
         size_t found    = 0;
   };
}
#include "../../../_util.undef.h"