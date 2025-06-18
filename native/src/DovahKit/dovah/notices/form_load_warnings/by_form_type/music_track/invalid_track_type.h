#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::music_track {
   class invalid_track_type final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr invalid_track_type(form_stub& subject, uint32_t v) : base_form_load_warning(subject), seen_type(v) {}

         uint32_t seen_type;
   };
}
#include "../../../_util.undef.h"