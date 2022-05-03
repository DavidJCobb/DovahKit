#version 450
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_multiview : enable

#include "../../includes/structs/rendered_light_shader_params.glsl"
#include "../../includes/structs/rendered_mesh_push_constant.glsl"
#include "../../includes/structs/rendered_mesh_shader_params.glsl"
#include "../../includes/structs/scene_global_state.glsl"

#include "../../includes/standard_vertex_inputs.glsl"

layout (constant_id = 0) const int CASTER_INDEX = 0;
layout (constant_id = 2) const int MAX_LIGHTS   = 4;

#include "shadows-caster.descriptors.glsl"
DECLARE_SCENE_STATE_PARAMS
DECLARE_ALL_MESHES_PARAMS
DECLARE_ALL_LIGHTS_PARAMS
DECLARE_SHADOW_CASTER_MAP_RENDER_PARAMS

layout(location = 0) out VS_OUT {
   vec2  uv;
   float distance;
} vs_out;

out gl_PerVertex {
   vec4 gl_Position;
};

void main() {
   mat4 model_transform = scene_meshes[pushed.object_index].transform;
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
	gl_Position = (light_space * model_transform) * vec4(in_position, 1.0);
   vs_out.uv   = in_uv;
   vs_out.distance = length((model_transform) * vec4(in_position, 1.0) - light_pos) / light_radius;
}