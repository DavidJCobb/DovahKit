#pragma once
#include "../common_fundamental.h"
#include "../../../../../data/collision_layers.h"

namespace dovah::loaded_forms::components::extra_data_types {
   // The value is a collision layer ID. See `enum class dovah::collision_layer`.
   class collision_data : public common_fundamental<collision_data, 'XTRI', uint32_t> {
      public:
         constexpr collision_layer get_layer_id() const noexcept { return (collision_layer)this->value; };
         constexpr void set_layer_id(collision_layer l) noexcept { this->value = (uint32_t)l; }
   };
}