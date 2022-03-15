#version 450

layout(push_constant) uniform PER_OBJECT {
   int   object_index;
	int   texture_index;
   int   texture_normal_index;
   float alpha_test_threshold;
   int   alpha_test_operation;
   bool  enable_alpha_blending;
} pushed;

struct ObjectData {
	mat4  transform;
   vec3  specular_color;
   float specular_strength;
   float specular_exponent;
};

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_color;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in vec3 in_normal;
layout(location = 4) in vec3 in_tangent;
layout(location = 5) in vec3 in_bitangent;

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

out gl_PerVertex {
   vec4 gl_Position;   
};

void main() {
   mat4 model_transform = objectBuffer.objects[pushed.object_index].transform;
	gl_Position = (ubo.sun_space * model_transform) * vec4(in_position, 1.0);
}