#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::perk {
   class effect_entry_point_params_specify_an_invalid_av : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr effect_entry_point_params_specify_an_invalid_av(
            form_stub& stub,
            size_t which_effect,
            float av_seen
         )
         :
            base_form_load_warning(stub),
            which_effect(which_effect),
            av_seen(av_seen)
         {}

         size_t which_effect;
         float av_seen;
   };
}
#include "../../../_util.undef.h"