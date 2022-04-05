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

// binding 0 varies between sun shadows and light shadows
// binding 1: rendered_mesh_shader_params[] (not used here)
layout(binding = 2) uniform sampler texSampler;
layout(binding = 3) uniform texture2D textures[];

layout(location = 0) in VS_OUT {
   vec2 uv;
} fs_in;

void main() {
   vec4 color = texture(sampler2D(textures[pushed.texture_index], texSampler), fs_in.uv);
   alpha_testing_conditional_discard(color.a, pushed.alpha_test_operation, pushed.alpha_test_threshold);
}