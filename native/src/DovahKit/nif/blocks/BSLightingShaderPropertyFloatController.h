#pragma once
#include "NiFloatInterpController.h"

namespace nifDK::block_types {
   class BSLightingShaderPropertyFloatController : public NiFloatInterpController {
      public:
         static constexpr const char* const type_name = "BSLightingShaderPropertyFloatController";
      public:
         struct field {
            enum type : uint32_t {
               refraction_strength   =  0, // BSLightingShaderProperty::material::refraction
               environment_map_scale =  8, // BSLightingShaderProperty::texture::env_map_scale
               glossiness            =  9, // BSLightingShaderProperty::material::glossiness
               specular_strength     = 10, // BSLightingShaderProperty::specular::strength
               emissive_multiple     = 11, // BSLightingShaderProperty::emissive::rgb_mult
               alpha                 = 12, // BSLightingShaderProperty::material::alpha
               u_offset              = 20, // BSLightingShaderProperty::texture::uv_offset.x
               u_scale               = 21, // BSLightingShaderProperty::texture::uv_scale.x
               v_offset              = 22, // BSLightingShaderProperty::texture::uv_offset.y
               v_scale               = 23, // BSLightingShaderProperty::texture::uv_scale.y
            };
         };
         using field_t = std::underlying_type_t<field::type>;

         field_t field = field::refraction_strength;

         virtual void parse(file_reader&) override;
   };
}