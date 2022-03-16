#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : enable

#include "includes/alpha_testing_conditional_discard.glsl"

layout(push_constant) uniform PER_OBJECT {
   int   object_index;
	int   texture_index;
   int   texture_normal_index;
   float alpha_test_threshold;
   int   alpha_test_operation;
   int   enable_alpha_blending; // VkBool32
} pushed;

struct ObjectData {
	mat4  transform;
   vec3  specular_color;
   float specular_strength;
   float specular_exponent;
};

layout(std140,binding = 0) uniform UniformBufferObject {
   mat4 view;
   mat4 proj;
   vec3 ambient_light_color;
   vec3 sun_dir;
   vec3 sun_color;
	mat4 sun_space;
} ubo;
layout(std430,set = 0, binding = 1) readonly buffer ObjectBuffer {
	ObjectData objects[];
} objectBuffer;
layout(binding = 2) uniform sampler texSampler;
layout(binding = 3) uniform texture2D textures[];

layout(location = 0) in VS_OUT {
   vec2 uv;
} fs_in;

void main() {
   ObjectData current_object = objectBuffer.objects[pushed.object_index];
   //
   vec4 color = texture(sampler2D(textures[pushed.texture_index], texSampler), fs_in.uv);
   alpha_testing_conditional_discard(color.a, pushed.alpha_test_operation, pushed.alpha_test_threshold);
}