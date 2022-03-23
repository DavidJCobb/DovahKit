#version 450
#extension GL_GOOGLE_include_directive : enable

#include "includes/point_light.glsl"
#include "includes/rendered_mesh_push_constant.glsl"
#include "includes/rendered_mesh_shader_params.glsl"
#include "includes/scene_global_state.glsl"

#include "includes/standard_vertex_inputs.glsl"

layout (constant_id = 0) const int   CASTER_INDEX = 0;
layout (constant_id = 1) const float FLIP_YAW     = 0;
layout (constant_id = 2) const int   MAX_LIGHTS   = 4;

layout(std140,binding = 0) uniform UniformBufferObject {
   scene_global_state ubo;
};
layout(std430,set = 0, binding = 1) readonly buffer ObjectBuffer {
	rendered_mesh_shader_params objects[];
} objectBuffer;
layout(std140,set = 0, binding = 2) readonly buffer PointLightBuffer {
	point_light lights[MAX_LIGHTS];
} pointLightBuffer;
// binding 3: texture sampler
// binding 4: texture array

layout(location = 0) out VS_OUT {
   vec2 uv;
} vs_out;

void main() {
   mat4 model_transform = objectBuffer.objects[pushed.object_index].transform;
   mat4 light_space;
   if (FLIP_YAW == 0) {
      light_space = ubo.shadow_caster_space_pos[CASTER_INDEX];
   } else {
      light_space = ubo.shadow_caster_space_neg[CASTER_INDEX];
   }
   //
	gl_Position = (light_space * model_transform) * vec4(in_position, 1.0);
   vs_out.uv   = in_uv;
}