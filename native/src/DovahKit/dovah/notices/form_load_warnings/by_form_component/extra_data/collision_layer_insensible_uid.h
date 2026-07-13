#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_component::extra_data {
   class collision_layer_insensible_uid : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

      public:
         constexpr collision_layer_insensible_uid(
            form_stub& subject,
            uint32_t uid
         )
         :
            base_form_load_warning(subject),
            uid(uid)
         {}

         uint32_t uid;
   };
}
#include "../../../_util.undef.h"