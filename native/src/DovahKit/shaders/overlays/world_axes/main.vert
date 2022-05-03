#version 450

layout(std140,binding = 0) uniform UniformBufferObject {
   mat4 view;
   mat4 proj;
} ubo;

layout(location = 0) in vec3  in_position;
layout(location = 1) in vec3  in_color;
layout(location = 2) in float in_radius;

layout(location = 0) out VS_OUT {
   vec3 color;
} vs_out;

void main() {
   gl_Position  = ubo.proj * ubo.view * vec4(in_position, 1.0);
   gl_PointSize = in_radius;
   //
   vs_out.color = in_color;
}