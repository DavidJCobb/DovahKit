#include "BSLightingShaderProperty.h"
#include "../reader.h"

#include "BSShaderTextureSet.h"

namespace nifDK::block_types {
   void BSLightingShaderProperty::parse(file_reader& reader) {
      BSShaderProperty::parse(reader);
      //
      {  // NIF format is cursed; one of BSLightingShaderProperty's fields comes before superclass NiObjectNET's fields
         auto* p = (shader_type*) reader.data_at(0);
         this->type = *p;
      }
      //
      reader.read(this->shader_flags);
      reader.read(this->texture.uv_offset);
      reader.read(this->texture.uv_scale);
      reader.read_ref(this->texture.paths);
      reader.read(this->emissive.color);
      reader.read(this->emissive.rgb_mult);
      if (reader.user_version<2>() == 130) {
         reader.read_indexed_string(this->wetness.material);
      }
      reader.read(this->texture.clamp_mode);
      reader.read(this->material.alpha);
      reader.read(this->material.refraction);
      reader.read(this->material.glossiness);
      reader.read(this->material.smoothness);
      reader.read(this->specular.color);
      reader.read(this->specular.strength);
      if (reader.user_version<2>() < 130) {
         reader.read(this->lighting_effect_strength);
      }
      if (reader.user_version<2>() == 130) {
         reader.read(this->subsurface_rolloff);
         reader.read(this->rim_light_power);
         if (this->rim_light_power == FLT_MAX)
            reader.read(this->backlight_power);
         reader.read(this->greyscale_to_palette_scale);
         reader.read(this->wetness.specular.scale);
         reader.read(this->wetness.specular.power);
         reader.read(this->wetness.min_var);
         reader.read(this->wetness.env_map_scale);
         reader.read(this->wetness.fresnel_power);
      }
      if (this->type == shader_type::environment_map) {
         reader.read(this->environment_map.scale);
         if (reader.user_version<2>() == 130) {
            reader.read(this->environment_map.unknown);
         }
      }
      if (this->type == shader_type::skin) {
         reader.read(this->skin.tint_color);
         if (reader.user_version<2>() == 130) {
            reader.read(this->skin.unknown);
         }
      }
      if (this->type == shader_type::hair) {
         reader.read(this->hair.tint_color);
      }
      if (this->type == shader_type::parallax_occlusion) {
         reader.read(this->parallax.single.scale);
      }
      if (this->type == shader_type::multilayer_parallax) {
         reader.read(this->parallax.multilayer.inner_layer_thickness);
         reader.read(this->parallax.multilayer.refraction_scale);
         reader.read(this->parallax.multilayer.inner_layer_texture_scale);
         reader.read(this->parallax.multilayer.environment_map_strength);
      }
      if (this->type == shader_type::sparkle_snow) {
         reader.read(this->sparkle_snow.parameters);
      }
      if (this->type == shader_type::eye_environment_map) {
         reader.read(this->eye_environment_map.cubemap_scale);
         reader.read(this->eye_environment_map.reflection_centers.left_eye);
         reader.read(this->eye_environment_map.reflection_centers.right_eye);
      }
   }
}