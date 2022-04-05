#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : enable

//
// The corresponding vertex shader is  sun-shadow-depth.vert;  the same descriptors are 
// used for both.
//
// This shader produces a linear depth value,  and is meant to be used with a MIN blend 
// mode.  Its purpose is to work around  a problem with using a depth buffer:  the math 
// needed for a typical view/projection matrix to get meshes onto the right "on-screen" 
// positions will not also produce linear depth,  or useful depth values in general. In 
// my tests, values were often in the range [0.999, 1.000] or, with inverted depth, the 
// range [0.000, 0.001]. Maybe a mathematician could design a projection matrix that'll 
// produce correct "on-screen" XY-coordinates  as well as usable Z-coordinates, but I'm 
// not a mathematician.
//
// So, we just use a color attachment with  one channel whose pixels are, functionally, 
// just depth values normalized to [0, 1].
//

#include "includes/alpha_testing_conditional_discard.glsl"

#include "includes/rendered_mesh_push_constant.glsl"
#include "includes/rendered_mesh_shader_params.glsl"
#include "includes/scene_global_state.glsl"

// binding 0: scene_global_state (not needed in fragment shader)
// binding 1: rendered_mesh_shader_params[] (not needed in fragment shader)
// binding 2: all_rendered_lights  (not needed in fragment shader)
// binding 3: light space matrices (not needed in fragment shader)
layout(binding = 4) uniform sampler texSampler;
layout(binding = 5) uniform texture2D textures[];

layout(location = 0) in VS_OUT {
   vec2  uv;
   float distance;
} fs_in;

layout(location = 0) out float out_color;

void main() {
   if (pushed.alpha_test_operation != 0) { // != GL_ALWAYS
      vec4 color = texture(sampler2D(textures[pushed.texture_index], texSampler), fs_in.uv);
      alpha_testing_conditional_discard(color.a, pushed.alpha_test_operation, pushed.alpha_test_threshold);
   }
   out_color = fs_in.distance;
}