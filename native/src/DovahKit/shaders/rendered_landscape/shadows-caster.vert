#version 450
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_multiview : enable

#include "../includes/structs/rendered_light_shader_params.glsl"
#include "../includes/structs/rendered_landscape_shader_params.glsl"
#include "../includes/structs/scene_global_state.glsl"

#include "vertex-inputs.glsl"

layout (constant_id = 0) const int CASTER_INDEX = 0;
layout (constant_id = 2) const int MAX_LIGHTS   = 4;

#include "shadows-caster.descriptors.glsl"
DECLARE_SCENE_STATE_PARAMS
DECLARE_ALL_LANDSCAPES_PARAMS
DECLARE_ALL_LIGHTS_PARAMS
DECLARE_SHADOW_CASTER_MAP_RENDER_PARAMS

layout(location = 0) out VS_OUT {
   float distance;
} vs_out;

out gl_PerVertex {
   vec4 gl_Position;
};

#include "functions/calc_local_vertex_position.glsl"

void main() {
   int quad = gl_InstanceIndex % 4;
   int landscape_index = gl_InstanceIndex / 4;

   vec3 pos_world = calc_local_vertex_position(quad, gl_VertexIndex, in_height) + landscapes[landscape_index].position;
   //
   vec4  light_pos;
   float light_radius = 1;
   {
      int light_index = scene.shadow_caster_index[CASTER_INDEX];
      if (light_index >= 0) {
         light_pos    = scene_lights[light_index].transform[3];
         light_radius = scene_lights[light_index].radius;
      }
   }
   mat4 light_space = light_space_matrices[CASTER_INDEX][gl_ViewIndex];
   //
	gl_Position = (light_space) * vec4(pos_world, 1.0);
   vs_out.distance = length(vec4(pos_world, 1.0) - light_pos) / light_radius;
}