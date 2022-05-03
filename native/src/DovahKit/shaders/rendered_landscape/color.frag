#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : enable

#include "../includes/structs/scene_global_state.glsl"
#include "../includes/structs/rendered_landscape_shader_params.glsl"

#include "../includes/descriptor_sets/all_landscapes.glsl"
#include "../includes/descriptor_sets/all_textures.glsl"
#include "../includes/descriptor_sets/scene_state.glsl"

#define SET_SCENE_STATE    0
#define SET_ALL_TEXTURES   1
#define SET_ALL_LANDSCAPES 2
#define SET_ALL_LIGHTS     3

DECLARE_SCENE_STATE_PARAMS    // scene
DECLARE_ALL_TEXTURES_PARAMS   // default_sampler; textures[]
DECLARE_ALL_LANDSCAPES_PARAMS // landscapes[]

layout(location = 0) in VS_OUT {
   vec4     in_color;
   vec3     in_world_pos;
   flat int in_quad;
   float    in_blends[6];
   vec2     in_uv;
   flat int in_landscape_index;
};

layout(location = 0) out vec4 out_color;

#define current_landscape landscapes[in_landscape_index]

void main() {
   out_color = in_color; // vertex color
   //
   // Apply texture blending:
   //
   vec3 base_color;
   if (current_landscape.diffuse_base[in_quad] >= 0) {
      base_color = texture(sampler2D(textures[current_landscape.diffuse_base[in_quad]], default_sampler), in_uv).rgb;
   } else {
      base_color = vec3(1, 1, 1);
   }
   //
   vec3 base_tex_normal;
   if (current_landscape.normals_base[in_quad] >= 0) {
      base_tex_normal = texture(sampler2D(textures[current_landscape.normals_base[in_quad]], default_sampler), in_uv).rgb;
   } else {
      base_tex_normal = vec3(0, 0, 1);
   }
   //
   vec3 tex_color  = base_color;
   vec3 tex_normal = base_tex_normal;
   for(int i = 0; i < 6; ++i) {
      float blend_alpha = in_blends[i];
      if (blend_alpha > 0) {
         int index_index   = (in_quad * 6) + i;
         int diffuse_index = current_landscape.diffuse_blends[index_index];
         int normals_index = current_landscape.normals_blends[index_index];
         //
         vec3 current_color;
         vec3 current_tex_normal;
         if (diffuse_index >= 0) {
            current_color = texture(sampler2D(textures[diffuse_index], default_sampler), in_uv).rgb;
         } else {
            // TODO: this is wrong; use the Skyrim.ini-specified default textures instead ([Landscape]sDefaultLandDiffuseTexture)
            current_color = base_color;
         }
         if (normals_index >= 0) {
            current_tex_normal = texture(sampler2D(textures[normals_index], default_sampler), in_uv).rgb;
         } else {
            // TODO: this is wrong; use the Skyrim.ini-specified default textures instead ([Landscape]sDefaultLandNormalTexture)
            current_tex_normal = base_tex_normal;
         }
         //
         tex_color  = mix(tex_color,  current_color,      blend_alpha);
         tex_normal = mix(tex_normal, current_tex_normal, blend_alpha);
      }
   }
   //
   // Apply vertex color to texture color:
   //
   out_color.rgb *= tex_color;
   out_color.a = 1;
}