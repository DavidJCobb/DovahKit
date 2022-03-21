#version 450
#extension GL_GOOGLE_include_directive : enable

#include "includes/rendered_mesh_push_constant.glsl"
#include "includes/rendered_mesh_shader_params.glsl"
#include "includes/scene_global_state.glsl"

#include "includes/standard_vertex_inputs.glsl"

layout(std140,binding = 0) uniform UniformBufferObject {
   scene_global_state ubo;
};
layout(std430,set = 0, binding = 1) readonly buffer ObjectBuffer {
	rendered_mesh_shader_params objects[];
} objectBuffer;
layout(binding = 2) uniform sampler texSampler;
layout(binding = 3) uniform texture2D textures[];

layout(location = 0) out VS_OUT {
   vec2 uv;
} vs_out;

void main() {
   mat4 model_transform = objectBuffer.objects[pushed.object_index].transform;
   //
	gl_Position = (ubo.sun_space * model_transform) * vec4(in_position, 1.0);
   vs_out.uv   = in_uv;
}