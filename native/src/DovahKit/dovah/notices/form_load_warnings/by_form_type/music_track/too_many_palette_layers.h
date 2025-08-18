#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::music_track {
   class too_many_palette_layers : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr too_many_palette_layers(
            form_stub& stub,
            size_t     count,
            size_t     max_count
         )
         :
            base_form_load_warning(stub),
            count(count),
            max_count(max_count)
         {}

         size_t count;
         size_t max_count;
   };
}
#include "../../../_util.undef.h"