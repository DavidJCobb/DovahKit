#pragma once
#include <string>
#include "../../../helpers/vector3.h"
#include "../_common.h"

namespace dovah::loaded_forms::components {
   struct object_bounds {
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
      void load(tes_subrecord_reader&);
      static void generate_use_info(tes_subrecord_reader&, form_stub_use_info_builder&);
      void save(tes_subrecord_writer&); // open the subrecord before calling
   };
}