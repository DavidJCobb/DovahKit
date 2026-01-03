#include "./directional_ambient_lighting_colors.h"
#include "../_common_cpp.h"
#include "./color_floats.h"

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

   void directional_ambient_lighting_colors::set_from_ambient(color_t ambient) {
      color_floats amb_f;
      {
         amb_f.r = (float)ambient.r / 255.0F;
         amb_f.g = (float)ambient.g / 255.0F;
         amb_f.b = (float)ambient.b / 255.0F;
      }

      constexpr const color_t x_pos_mult = { 141, 128, 134 };
      constexpr const color_t x_neg_mult = { 115, 128, 122 };
      constexpr const color_t y_pos_mult = { 128, 134, 128 };
      constexpr const color_t y_neg_mult = { 128, 122, 128 };
      constexpr const color_t z_pos_mult = {  61,  61,  58 };
      constexpr const color_t z_neg_mult = { 195, 195,  96 };

      auto _blend = [](const color_floats& a, const color_t& b) -> color_t {
         color_t out;
         out.r = (float)b.r * a.r / 128.0F;
         out.g = (float)b.g * a.g / 128.0F;
         out.b = (float)b.b * a.b / 128.0F;
         return out;
      };
      this->x.positive = _blend(amb_f, x_pos_mult);
      this->x.negative = _blend(amb_f, x_neg_mult);
      this->y.positive = _blend(amb_f, y_pos_mult);
      this->y.negative = _blend(amb_f, y_neg_mult);
      this->z.positive = _blend(amb_f, z_pos_mult);
      this->z.negative = _blend(amb_f, z_neg_mult);
   }
}