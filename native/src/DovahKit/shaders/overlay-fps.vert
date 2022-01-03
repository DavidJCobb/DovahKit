#version 450

const vec3 TEXT_COLOR = { 1.0, 1.0, 1.0 };

layout(set = 1, binding = 0) uniform UniformBufferObject {
   float view_w;
   float view_h;
} ubo;
// set = 1, binding 1 is used by the fragment shader (texture and sampler)

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
   gl_Position  = vec3(inPosition.x / ubo.view_w, inPosition.y / ubo.view_h, inPosition.z, 1.0);
   fragColor    = TEXT_COLOR;
   fragTexCoord = inTexCoord;
}