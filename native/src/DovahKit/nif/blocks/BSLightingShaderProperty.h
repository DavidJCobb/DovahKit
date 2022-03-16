#pragma once
#include <array>
#include "helpers/enum_flags.h"
#include "BSShaderProperty.h"
#include "../types/NiColor.h"
#include "../types/SkyrimShaderPropertyFlags.h"

namespace nifDK::block_types {
   class BSShaderTextureSet;

   class BSLightingShaderProperty : public BSShaderProperty {
      public:
         static constexpr const char* const type_name = "BSLightingShaderProperty";
      public:
         enum class shader_type {
            standard, // default
            environment_map,
            glow,
            parallax,
            face,
            skin,
            hair,
            parallax_occlusion,
            multitexture_landscape,
            lod_landscape,
            snow,
            multilayer_parallax,
            tree,
            lod_object,
            sparkle_snow,
            lod_object_hd,
            eye_environment_map,
            cloud,
            lod_landscape_noise,
            multitexture_landscape_lod_blend,
            fallout_4_dismemberment = 20,
         };
         enum class texture_clamp_mode : uint32_t {
            clamp_both     = 0,
            clamp_u_wrap_v = 1,
            wrap_u_clamp_v = 2,
            wrap_both      = 3,
         };

         shader_type type = shader_type::standard; // at data offset 0, i.e. before the contents of NiObjectNET
         std::array<SkyrimShaderPropertyFlags, 2> shader_flags = {};
         struct {
            glm::fvec2 uv_offset;
            glm::fvec2 uv_scale;
            texture_clamp_mode  clamp_mode = texture_clamp_mode::wrap_both;
            BSShaderTextureSet* paths;
            float env_map_scale;
         } texture;
         struct {
            NiColor color    = { 0, 0, 0 };
            float   rgb_mult = 1.0;
         } emissive;
         struct {
            float alpha      =  1.0; // [0, 1]
            float refraction =  0.0; // [0, 1]
            float glossiness = 80.0; //        // user version 2 <  130
            float smoothness =  1.0; // [0, 1] // user version 2 == 130
         } material;
         struct {
            NiColor color;
            float   strength = 1.0; // [0, 999]
         } specular;
         std::array<float, 2> lighting_effect_strength; // envmap, backlight, rimlight, etc.
         float subsurface_rolloff = 0.3;     // user version 2 == 130
         float rim_light_power    = FLT_MAX; // user version 2 == 130
         float backlight_power;              // user version 2 == 130 and (rim_light_power == FLT_MAX)
         float greyscale_to_palette_scale;   // user version 2 == 130
         float fresnel_power      = 5.0;     // user version 2 == 130
         struct {
            std::string material;
            struct {
               float scale = -1.0;
               float power = -1.0;
            } specular;
            float min_var        = -1.0;
            float env_map_scale  = -1.0;
            float fresnel_power  = -1.0;
            float metallic_scale = -1.0;
         } wetness;
         struct {
            float    scale = 1.0;
            uint16_t unknown; // user version 2 == 130
         } environment_map; // shader type == 1
         struct {
            NiColor  tint_color;
            uint32_t unknown;
         } skin; // shader type == 5
         struct {
            NiColor tint_color;
         } hair; // shader type == 6
         struct {
            struct {
               float scale;
            } single; // shader type == 7
            struct {
               float      inner_layer_thickness;
               float      refraction_scale;
               glm::fvec2 inner_layer_texture_scale;
               float      environment_map_strength;
            } multilayer; // shader type == 11 (multilayer_parallax)
         } parallax;
         struct {
            glm::fvec4 parameters;
         } sparkle_snow; // shader type == 14
         struct {
            float cubemap_scale;
            struct {
               glm::fvec3 left_eye;
               glm::fvec3 right_eye;
            } reflection_centers;
         } eye_environment_map; // shader type == 16

         virtual void parse(file_reader&) override;
   };
}