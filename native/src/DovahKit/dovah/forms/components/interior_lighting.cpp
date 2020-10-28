#include "interior_lighting.h"
#include "../_common_cpp.h"

namespace dovah::loaded_forms::components {
   void interior_lighting::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      if (subrecord.is_in_bounds(0x40)) {
         this->ambient.load(subrecord);
         this->directional.load(subrecord);
         this->fog_color_near.load(subrecord);
         subrecord.read(this->fog_distance_near);
         subrecord.read(this->fog_distance_far);
         subrecord.read(this->rotation.xy);
         subrecord.read(this->rotation.z);
         subrecord.read(this->directional_fade);
         subrecord.read(this->fog_distance_clip);
         subrecord.read(this->fog_power);
         this->directional_ambient_colors.x_pos.load(subrecord);
         this->directional_ambient_colors.x_neg.load(subrecord);
         this->directional_ambient_colors.y_pos.load(subrecord);
         this->directional_ambient_colors.y_neg.load(subrecord);
         this->directional_ambient_colors.z_pos.load(subrecord);
         this->directional_ambient_colors.z_neg.load(subrecord);
      }
      if (!subrecord.is_in_bounds()) // per UESP, NavMeshGenCellDUPLICATE001 only has the first 0x40 bytes of this struct
         return;
      this->specular.load(subrecord);
      subrecord.read(this->fresnel);
      this->fog_color_far.load(subrecord);
      subrecord.read(this->fog_max);
      subrecord.read(this->light_fade_distance.start);
      subrecord.read(this->light_fade_distance.end);
      subrecord.read(this->inherit_flags);
   }
   /*static*/ void interior_lighting::generate_use_info(tes_subrecord_reader& subrecord, form_stub_use_info_builder& uib) {
      return;
   }
   void interior_lighting::save(tes_subrecord_writer& subrecord) {
      this->ambient.save(subrecord);
      this->directional.save(subrecord);
      this->fog_color_near.save(subrecord);
      subrecord.write(this->fog_distance_near);
      subrecord.write(this->fog_distance_far);
      subrecord.write(this->rotation.xy);
      subrecord.write(this->rotation.z);
      subrecord.write(this->directional_fade);
      subrecord.write(this->fog_distance_clip);
      subrecord.write(this->fog_power);
      this->directional_ambient_colors.x_pos.save(subrecord);
      this->directional_ambient_colors.x_neg.save(subrecord);
      this->directional_ambient_colors.y_pos.save(subrecord);
      this->directional_ambient_colors.y_neg.save(subrecord);
      this->directional_ambient_colors.z_pos.save(subrecord);
      this->directional_ambient_colors.z_neg.save(subrecord);
      this->specular.save(subrecord);
      subrecord.write(this->fresnel);
      this->fog_color_far.save(subrecord);
      subrecord.write(this->fog_max);
      subrecord.write(this->light_fade_distance.start);
      subrecord.write(this->light_fade_distance.end);
      subrecord.write(this->inherit_flags);
   }
}