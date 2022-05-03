#version 450

const vec3 TEXT_COLOR = { 1.0, 1.0, 1.0 };

layout(binding = 0) uniform UniformBufferObject {
   float view_w;
   float view_h;
} ubo;
// binding 1 is used by the fragment shader (texture and sampler)

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

float _to_clip_space(float screen_coord, float screen_dimension) {
   return 2 * (screen_coord / screen_dimension) - 1.0;
}

void main() {
   gl_Position  = vec4(_to_clip_space(inPosition.x, ubo.view_w), _to_clip_space(inPosition.y, ubo.view_h), inPosition.z, 1.0);
   fragColor    = TEXT_COLOR;
   fragTexCoord = inTexCoord;
}