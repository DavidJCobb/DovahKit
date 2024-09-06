#pragma once
#include <array>
#include "../_common.h"

namespace dovah::loaded_forms::structs {
   struct movement_type_speeds {
      public:
         struct speed_set {
            float walk;
            float run;
         };

      public:
         union {
            std::array<float, 11> list = {};
            struct {
               speed_set left;
               speed_set right;
               speed_set forward;
               speed_set back;
               speed_set rotate_in_place; // radians
               float     rotate_while_moving; // run only, no walk // radians // added in form version 27
            };
         };
         
         void load(tes_subrecord_reader&, load_order_interfaces::form_load&);
         void save(tes_subrecord_writer&, load_order_interfaces::form_save&) const;
   };

}