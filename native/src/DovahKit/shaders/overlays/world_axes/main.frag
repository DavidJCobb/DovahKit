#version 450

layout(std140,binding = 0) uniform UniformBufferObject {
   mat4 view;
   mat4 proj;
} ubo;

layout(location = 0) in VS_OUT {
   vec3 color;
} fs_in;

layout(location = 0) out vec4 outColor;

void main() {
   outColor = vec4(fs_in.color, 1);
}