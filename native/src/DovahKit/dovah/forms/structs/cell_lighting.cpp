#include "cell_lighting.h"
#include "../_common_cpp.h"

namespace dovah::loaded_forms::structs {
   void cell_lighting::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      this->ambient.base.load(subrecord);
      this->directional.color.load(subrecord);
      this->fog.colors.near.load(subrecord);
      subrecord.read(this->fog.near);
      subrecord.read(this->fog.far);
      subrecord.read(this->directional.rotation.xy);
      subrecord.read(this->directional.rotation.z);
      subrecord.read(this->directional.fade);
      subrecord.read(this->fog.clip_distance);
      subrecord.read(this->fog.power);
      this->ambient.directional.load(subrecord);
      if (!subrecord.is_in_bounds()) // per UESP, NavMeshGenCellDUPLICATE001 only has the first 0x40 bytes of this struct
         return;
      this->fog.colors.far.load(subrecord);
      subrecord.read(this->fog.max);
      subrecord.read(this->light_fade_distance.start);
      subrecord.read(this->light_fade_distance.end);
      if (subrecord.get_containing_record().version() >= 34) {
         subrecord.read(this->inherit_flags);
      }
   }
   void cell_lighting::save(tes_subrecord_writer& subrecord, load_order_interfaces::form_save& intfc) {
      this->ambient.base.save(subrecord);
      this->directional.color.save(subrecord);
      this->fog.colors.near.save(subrecord);
      subrecord.write(this->fog.near);
      subrecord.write(this->fog.far);
      subrecord.write(this->directional.rotation.xy);
      subrecord.write(this->directional.rotation.z);
      subrecord.write(this->directional.fade);
      subrecord.write(this->fog.clip_distance);
      subrecord.write(this->fog.power);
      this->ambient.directional.save(subrecord);
      this->fog.colors.far.save(subrecord);
      subrecord.write(this->fog.max);
      subrecord.write(this->light_fade_distance.start);
      subrecord.write(this->light_fade_distance.end);
      if (subrecord.get_containing_record().version() >= 34) {
         subrecord.write(this->inherit_flags);
      }
   }
}