#include "./directional_ambient_lighting_colors.h"
#include "../_common_cpp.h"

namespace dovah::loaded_forms::structs {
   void directional_ambient_lighting_colors::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      this->x.positive.load(subrecord);
      this->x.negative.load(subrecord);
      this->y.positive.load(subrecord);
      this->y.negative.load(subrecord);
      this->z.positive.load(subrecord);
      this->z.negative.load(subrecord);
      if (!subrecord.is_in_bounds()) // per UESP, NavMeshGenCellDUPLICATE001 only has the first 0x40 bytes of this struct
         return;
      this->specular.load(subrecord);
      subrecord.read(this->fresnel);
   }
   void directional_ambient_lighting_colors::save(tes_subrecord_writer& subrecord, load_order_interfaces::form_save& intfc) {
      this->x.positive.save(subrecord);
      this->x.negative.save(subrecord);
      this->y.positive.save(subrecord);
      this->y.negative.save(subrecord);
      this->z.positive.save(subrecord);
      this->z.negative.save(subrecord);
      this->specular.save(subrecord);
      subrecord.write(this->fresnel);
   }
}