#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::scene {
   class unterminated_scene_phase final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr unterminated_scene_phase(form_stub& subject, uint32_t id) : base_form_load_warning(subject), which_phase(id) {}

         uint32_t which_phase;
   };
}
#include "../../../_util.undef.h"