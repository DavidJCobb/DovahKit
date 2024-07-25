#pragma once
#include "../../../base_form_load_warning.h"

#include "dovah/data/face_fx/phonemes.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::race {
   class too_many_phonemes : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr too_many_phonemes(
            form_stub& stub,
            size_t     count
         )
         :
            base_form_load_warning(stub),
            count(count)
         {}

         size_t count;
         size_t max_count = dovah::face_fx::phoneme_count;
   };
}
#include "../../../_util.undef.h"