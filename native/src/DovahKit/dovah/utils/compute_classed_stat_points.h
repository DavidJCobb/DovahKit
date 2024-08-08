#pragma once
#include <array>
#include <cstdint>
#include "../data/skills.h"

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
      using value_type = uint32_t;

      std::array<value_type, skill_count> skill_points = { 0 };
      union _ {
         ~_() { list.~array(); }

         std::array<value_type, 3> list = { 0 };
         struct {
            value_type health;
            value_type magicka;
            value_type stamina;
         };
         struct {
            value_type h;
            value_type m;
            value_type s;
         };
      } attribute_points;
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