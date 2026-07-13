#pragma once
#include "../../../base_form_load_warning.h"
#include "dovah/data/collision_layers.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::collision_layer {
   class invalid_uid : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr invalid_uid(
            form_stub& stub,
            uint32_t uid
         )
         :
            base_form_load_warning(stub),
            uid(uid)
         {}

         uint32_t uid;

         constexpr const bool is_unsafe_uid() const noexcept {
            return this->uid > max_safe_collision_layer_uid;
         }
   };
}
#include "../../../_util.undef.h"