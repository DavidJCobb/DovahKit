#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::scene {
   class unexpected_subrecord_in_scene_phase final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr unexpected_subrecord_in_scene_phase(form_stub& subject, uint32_t id, uint32_t sig) : base_form_load_warning(subject), which_phase(id), signature(sig) {}

         uint32_t which_phase;
         uint32_t signature = 0;
   };
}
#include "../../../_util.undef.h"