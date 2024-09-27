#pragma once
#include <cstdint>
#include <vector>
#include "../_common.h"

namespace dovah::loaded_forms::structs {
   struct world_max_height_data {
      public:
         struct quad_heights {
            int8_t sw;
            int8_t se;
            int8_t nw;
            int8_t ne;
         };

      public:
         struct {
            int16_t x;
            int16_t y;
         } min;
         struct {
            int16_t x;
            int16_t y;
         } max;
         std::vector<quad_heights> cells;

      public:
         void load(tes_subrecord_reader&, load_order_interfaces::form_load&);
         void save(tes_subrecord_writer&, load_order_interfaces::form_save&) const;

         constexpr bool empty() const noexcept { return this->cells.empty(); }
   };
}