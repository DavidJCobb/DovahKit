#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : enable

#include "../includes/structs/scene_global_state.glsl"
#include "../includes/structs/rendered_landscape_shader_params.glsl"
#include "../includes/structs/rendered_light_shader_params.glsl"

#include "../includes/descriptor_sets/all_landscapes.glsl"
#include "../includes/descriptor_sets/all_lights.glsl"
#include "../includes/descriptor_sets/all_textures.glsl"
#include "../includes/descriptor_sets/scene_state.glsl"

#include "vertex-inputs.glsl"

#define SET_SCENE_STATE    0
#define SET_ALL_TEXTURES   1
#define SET_ALL_LANDSCAPES 2
#define SET_ALL_LIGHTS     3
#define SET_SHADOW_MAPS    4

DECLARE_SCENE_STATE_PARAMS    // scene
DECLARE_ALL_LANDSCAPES_PARAMS // landscapes[]
DECLARE_ALL_LIGHTS_PARAMS     // scene_lights[]

#include "color.fragment-input.glsl"
layout(location = 0) out VS_OUT {
   fragment_input vs_out;
   //
   // Can't use storage/interpolation qualifiers on struct members, so these've gotta 
   // hang out:
   //
   flat int vs_out_quad;
   flat int vs_out_landscape_index;
};

// [-1, 1] to [0, 1]
const mat4 shadow_to_normalized_coords = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0
);

#include "../includes/prep_all_light_and_shadow.glsl"
#include "functions/calc_local_vertex_position.glsl"

void main() {
   vs_out_quad = gl_InstanceIndex % 4;
   vs_out_landscape_index = gl_InstanceIndex / 4;

   vec3 pos_world = calc_local_vertex_position(vs_out_quad, gl_VertexIndex, in_height) + landscapes[vs_out_landscape_index].position;
   gl_Position = scene.proj * scene.view * vec4(pos_world, 1);
   //
   vs_out.color = vec4(in_color, 1);
   //
   for(int i = 0; i < 6; ++i)
      vs_out.blends[i] = blends[i];
   vs_out.uv = vec2(pos_world / 128);
   //
   // Lighting:
   //
   mat3 tangent_space;
   {
      vec3 t = vec3(1, 0, 0); // tangent
      vec3 b = vec3(0, 1, 0); // bitangent
      vec3 n;                 // normal
      t = cross(t, in_normal);
      b = cross(in_normal, t);
      t = normalize(t);
      b = normalize(b);
      n = normalize(in_normal);
      tangent_space = transpose(mat3(t, b, n));
   }
   vs_out.lighting_data = prep_all_light_and_shadow(
      pos_world,
      tangent_space
   );
}