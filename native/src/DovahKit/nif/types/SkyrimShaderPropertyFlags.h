#pragma once
#include <cstdint>
#include <type_traits>

namespace nifDK {
   using SkyrimShaderPropertyFlags = uint32_t;

   struct SkyrimShaderPropertyFlagA {
      SkyrimShaderPropertyFlagA() = delete;
      enum type : SkyrimShaderPropertyFlags {
         specular = 0x00000001,
         skinned = 0x00000002,
         temp_refraction = 0x00000004,
         vertex_alpha = 0x00000008,
         greyscale_to_palette_color = 0x00000010,
         greyscale_to_palette_alpha = 0x00000020,
         use_falloff = 0x00000040,
         environment_mapping = 0x00000080,
         receive_shadows = 0x00000100,
         cast_shadows = 0x00000200,
         facegen_detail_map = 0x00000400,
         parallax = 0x00000800,
         model_space_normals = 0x00001000,
         non_projective_shadows = 0x00002000,
         landscape = 0x00004000,
         normal_map_is_refraction = 0x00008000,
         fire_refraction = 0x00010000,
         eye_environment_mapping = 0x00020000, // requires eye shader + skinned
         hair_soft_lighting = 0x00040000,
         screendoor_alpha_fade = 0x00080000,
         hide_from_local_map = 0x00100000, // hides object and anything beneath it from the local map
         facegen_rgb_tint = 0x00200000,
         own_emit = 0x00400000,
         projected_uv = 0x00800000,
         multiple_textures = 0x01000000,
         remappable_textures = 0x02000000,
         decal = 0x04000000,
         dynamic_decal = 0x08000000,
         parallax_occlusion = 0x10000000,
         external_emittance = 0x20000000,
         soft_effect = 0x40000000,
         zbuffer_test = 0x80000000,
      };
   };
   struct SkyrimShaderPropertyFlagB {
      SkyrimShaderPropertyFlagB() = delete;
      enum type : SkyrimShaderPropertyFlags {
         zbuffer_write                 = 0x00000001,
         lod_landscape                 = 0x00000002,
         lod_objects                   = 0x00000004,
         no_fade                       = 0x00000008,
         double_sided                  = 0x00000010,
         vertex_colors                 = 0x00000020,
         glow_map                      = 0x00000040,
         assume_shadowmask             = 0x00000080,
         packed_tangent                = 0x00000100,
         multi_index_snow              = 0x00000200,
         vertex_lighting               = 0x00000400,
         uniform_scale                 = 0x00000800,
         fit_slope                     = 0x00001000,
         billboard                     = 0x00002000,
         no_lod_land_blend             = 0x00004000,
         env_map_light_fade            = 0x00008000,
         wireframe                     = 0x00010000, // only particles?
         weapon_blood                  = 0x00020000,
         hide_on_local_map             = 0x00040000, // hides object from local map, but not things occluded by it
         has_premultiplied_alpha       = 0x00080000,
         cloud_lod                     = 0x00100000,
         anisotropic_lighting          = 0x00200000, // hair only?
         no_transparency_multisampling = 0x00400000,
         //
         multilayer_parallax           = 0x01000000,
         soft_lighting                 = 0x02000000,
         rim_lighting                  = 0x04000000,
         backlighting                  = 0x08000000,
         //
         use_tree_animation            = 0x20000000,
         effect_lighting               = 0x40000000,
         hd_lod_objects                = 0x80000000,
      };
   };
}