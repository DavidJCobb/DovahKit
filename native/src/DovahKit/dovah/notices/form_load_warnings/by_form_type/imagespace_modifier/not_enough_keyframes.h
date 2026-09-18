#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::imagespace_modifier {
   class not_enough_keyframes : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr not_enough_keyframes(
            form_stub& subject,
            uint32_t   subrecord_signature,
            size_t     count_expected,
            size_t     count_loaded
         )
         :
            base_form_load_warning(subject),
            subrecord_signature(subrecord_signature),
            count_expected(count_expected),
            count_loaded(count_loaded)
         {}

         uint32_t subrecord_signature = 0;
         size_t   count_expected = 0;
         size_t   count_loaded   = 0;
   };
}
#include "../../../_util.undef.h"