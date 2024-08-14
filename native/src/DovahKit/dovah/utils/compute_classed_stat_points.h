#pragma once
#include <array>
#include <cstdint>
#include "../data/skills.h"
#include "./data_by_actor_attribute.h"

namespace dovah {
   namespace loaded_forms {
      class Class;
      class Race;
   }
   class file_load_order;
   class form_stub;
}

namespace dovah {
   struct classed_stat_points {
      public:
         using value_type     = uint32_t;
         using attribute_trio = data_by_actor_attribute<value_type>;

      public:
         struct {
            attribute_trio base;
            attribute_trio calculated;
         } attribute_points;
         struct {
            std::array<value_type, skill_count> base       = { 0 };
            std::array<value_type, skill_count> calculated = { 0 };
         } skill_points;
   };

   //
   // Compute skill points conferred by an actor's class and race. (We can't 
   // exclude the race, because there exist skill caps that apply to the total 
   // points per skill -- base value and racial bonuses included.)
   //
   extern classed_stat_points compute_classed_stat_points(
      const file_load_order&,
      const dovah::loaded_forms::Class*,
      const dovah::loaded_forms::Race*,
      size_t level
   );
}