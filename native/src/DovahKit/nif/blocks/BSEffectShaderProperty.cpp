#include "BSEffectShaderProperty.h"
#include "../reader.h"

namespace nifDK::block_types {
   void BSEffectShaderProperty::parse(file_reader& reader) {
      BSShaderProperty::parse(reader);
      //
      reader.require_size(24);
      reader.unchecked_read(this->shader_flags); // meanings differ if user version 2 == 130 (FO4 definitions in that case)
      reader.unchecked_read(this->texture.uv_offset);
      reader.unchecked_read(this->texture.uv_scale);
      reader.read_prefixed_string<uint32_t>(this->texture.path);
      reader.require_size(36);
      reader.unchecked_read(this->texture.clamp_mode);    // +1
      reader.unchecked_read(this->lighting_influence);    // +1
      reader.unchecked_read(this->env_map_min_lod);       // +1
      reader.unchecked_read(this->unknown);               // +1
      reader.unchecked_read(this->falloff.angle.start);   // +4
      reader.unchecked_read(this->falloff.angle.stop);    // +4
      reader.unchecked_read(this->falloff.opacity.start); // +4
      reader.unchecked_read(this->falloff.opacity.stop);  // +4
      reader.unchecked_read(this->emissive.color);        // +4
      reader.unchecked_read(this->emissive.rgb_mult);     // +4
      reader.unchecked_read(this->soft_falloff_depth);    // +4
      reader.read_prefixed_string<uint32_t>(this->texture.greyscale);
      if (reader.user_version<2>() == 130) {
         reader.read_prefixed_string<uint32_t>(this->texture.environment_map);
         reader.read_prefixed_string<uint32_t>(this->texture.normal);
         reader.read_prefixed_string<uint32_t>(this->texture.environment_mask);
         reader.read(this->texture.env_map_scale);
      }
   }
}