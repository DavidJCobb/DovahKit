#pragma once
#include <cstdint>
#include <optional>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::scene {
   class invalid_scene_action_type final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr invalid_scene_action_type(form_stub& subject, uint16_t type) : base_form_load_warning(subject), action_type(type) {}

         uint16_t action_type;
   };
}
#include "../../../_util.undef.h"