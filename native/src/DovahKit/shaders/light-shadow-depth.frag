#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : enable

//
// This shader produces no outputs. Its sole purpose is to discard fragments that are
// made transparent by a model's RGBA texture, when the model uses alpha testing, so 
// that those fragments don't affect the depth buffer.
//
// The corresponding vertex shader is sun-shadow-depth.vert; the same descriptors are 
// used for both.
//

#include "includes/alpha_testing_conditional_discard.glsl"

#include "includes/rendered_mesh_push_constant.glsl"
#include "includes/rendered_mesh_shader_params.glsl"
#include "includes/scene_global_state.glsl"

layout(std140,binding = 0) uniform UniformBufferObject {
   scene_global_state ubo;
};
layout(std430,set = 0, binding = 1) readonly buffer ObjectBuffer {
	rendered_mesh_shader_params objects[];
} objectBuffer;
// binding 2: all_rendered_lights  (not needed in fragment shader)
// binding 3: light space matrices (not needed in fragment shader)
layout(binding = 4) uniform sampler texSampler;
layout(binding = 5) uniform texture2D textures[];

layout(location = 0) in VS_OUT {
   vec2  uv;
   float distance;
} fs_in;

layout(location = 0) out float out_frag_color;

void main() {
   rendered_mesh_shader_params current_object = objectBuffer.objects[pushed.object_index];
   //
   vec4 color = texture(sampler2D(textures[pushed.texture_index], texSampler), fs_in.uv);
   alpha_testing_conditional_discard(color.a, pushed.alpha_test_operation, pushed.alpha_test_threshold);
   //
   out_frag_color = fs_in.distance;
}