#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : enable

#include "../includes/structs/computed_light.glsl"
#include "../includes/structs/scene_global_state.glsl"
#include "../includes/structs/rendered_landscape_shader_params.glsl"
#include "../includes/structs/rendered_light_shader_params.glsl"

#include "../includes/descriptor_sets/all_landscapes.glsl"
#include "../includes/descriptor_sets/all_lights.glsl"
#include "../includes/descriptor_sets/all_textures.glsl"
#include "../includes/descriptor_sets/scene_state.glsl"
#include "../includes/descriptor_sets/shadow_maps.glsl"

layout (constant_id = 0) const int MAX_LIGHTS = 4;
#define SHADOW_CASTER_COUNT 4

#define SET_SCENE_STATE    0
#define SET_ALL_TEXTURES   1
#define SET_ALL_LANDSCAPES 2
#define SET_ALL_LIGHTS     3
#define SET_SHADOW_MAPS    4

DECLARE_SCENE_STATE_PARAMS    // scene
DECLARE_ALL_TEXTURES_PARAMS   // default_sampler, textures[]
DECLARE_ALL_LANDSCAPES_PARAMS // landscapes[]
DECLARE_ALL_LIGHTS_PARAMS     // scene_lights
DECLARE_SHADOW_MAPS_PARAMS

#include "color.fragment-input.glsl"
layout(location = 0) in VS_OUT {
   fragment_input fs_in;
   //
   // Can't use storage/interpolation qualifiers on struct members, so these've gotta 
   // hang out:
   //
   flat int fs_in_quad;
   flat int fs_in_landscape_index;
};

layout(location = 0) out vec4 out_color;

#include "../includes/apply_scene_fog.glsl"
#include "../includes/calc_all_light_and_shadow.glsl"
#include "../includes/expand_light_and_shadow_inputs.glsl"

vec4 sample_diffuse(int texture_index) { // returns RGBA
   if (texture_index < 0) {
      //
      // This is a fallback to Skyrim.ini's [Landscape]sDefaultLandDiffuseTexture. The 
      // Creation Kit can serialize landscapes as being painted with texture index -1, 
      // which explicitly indicates that they should blend with the default texture.
      //
      texture_index = scene.default_land_diffuse_texture;
      if (texture_index < 0)
         //
         // No default texture loaded. Go with "missing texture purple."
         //
         return vec4(1, 0, 1, 1);
   }
   return texture(sampler2D(textures[texture_index], default_sampler), fs_in.uv);
}

vec4 sample_normals(int texture_index) { // returns RGBA
   if (texture_index < 0) {
      //
      // This is a fallback to Skyrim.ini's [Landscape]sDefaultLandNormalTexture. The 
      // Creation Kit can serialize landscapes as being painted with texture index -1, 
      // which explicitly indicates that they should blend with the default texture.
      //
      texture_index = scene.default_land_diffuse_texture;
      if (texture_index < 0)
         //
         // No default texture loaded. Go with vertical normals.
         //
         return vec4(0, 0, 1, 1);
   }
   return texture(sampler2D(textures[texture_index], default_sampler), fs_in.uv);
}

#define current_landscape landscapes[fs_in_landscape_index]

void main() {
   out_color = fs_in.color; // vertex color
   //
   // Apply texture blending:
   //
   vec3 base_color      = sample_diffuse(current_landscape.diffuse_base[fs_in_quad]).rgb;
   vec3 base_tex_normal = sample_normals(current_landscape.normals_base[fs_in_quad]).rgb;
   //
   vec3 tex_color  = base_color;
   vec3 tex_normal = base_tex_normal;
   for(int i = 0; i < 6; ++i) {
      float blend_alpha = fs_in.blends[i];
      if (blend_alpha > 0) {
         int index_index   = (fs_in_quad * 6) + i;
         int diffuse_index = current_landscape.diffuse_blends[index_index];
         int normals_index = current_landscape.normals_blends[index_index];
         //
         vec3 current_color      = sample_diffuse(diffuse_index).rgb;
         vec3 current_tex_normal = sample_normals(normals_index).rgb;
         //
         tex_color  = mix(tex_color,  current_color,      blend_alpha);
         tex_normal = mix(tex_normal, current_tex_normal, blend_alpha);
      }
   }
   tex_normal = normalize(tex_normal);
   if (dot(tex_normal, tex_normal) == 0) // length-squared is zero?
      tex_normal = vec3(0, 0, 1);
   //
   // Apply vertex color to texture color:
   //
   out_color.rgb *= tex_color;
   out_color.a = 1;
   //
   // Lighting:
   //
   tex_normal = normalize(tex_normal * 2.0 - 1.0);
   {
      computed_light light_data = calc_all_light_and_shadow(
         expand_light_and_shadow_inputs(fs_in.lighting_data),
         1,             // receive_shadows
         vec3(1, 1, 1), // specular_color
         1.0,           // specular_exponent // TODO: load specular exponent from land textures?
         0.0,           // specular_strength
         tex_normal
      );
      out_color.rgb *= scene.ambient_light_color + light_data.diffuse + light_data.specular;
   }
   //
   apply_scene_fog(
      out_color,
      distance(fs_in.lighting_data.pos_world, scene.camera_pos)
   );
}