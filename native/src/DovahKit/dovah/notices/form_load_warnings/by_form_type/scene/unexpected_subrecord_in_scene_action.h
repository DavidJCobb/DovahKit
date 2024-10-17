#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::scene {
   class unexpected_subrecord_in_scene_action final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr unexpected_subrecord_in_scene_action(form_stub& subject, uint32_t id, uint32_t sig) : base_form_load_warning(subject), action_id(id), signature(sig) {}

         uint32_t action_id;
         uint32_t signature = 0;
   };
}
#include "../../../_util.undef.h"