#pragma once
#include <cstdint>
#include <string>
#include "../../../helpers/vector3.h"
#include "../_common.h"

namespace dovah::loaded_forms::components {
   struct object_bounds {
      static constexpr const uint32_t subrecord = 'OBND';

      using point_type = cobb::vector3<int16_t>;

      point_type min;
      point_type max;
      //
      void  get_size(point_type& out) const noexcept {
         out = this->max - this->min;
      }
      float get_volume() const noexcept {
         point_type sizes;
         this->get_size(sizes);
         return (float)sizes.x * (float)sizes.y * (float)sizes.z;
      }
      //
      void load(tes_subrecord_reader&, load_order_interfaces::form_load& intfc);
      static void generate_use_info(tes_subrecord_reader&, form_stub_use_info_builder&);
      void save(tes_subrecord_writer&, load_order_interfaces::form_save& intfc); // open the subrecord before calling
      void clear() noexcept;

      inline bool is_zero() const noexcept {
         return ((int)min.x + min.y + min.z + max.x + max.y + max.z) == 0;
      }
   };
}