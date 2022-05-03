#version 450

// binding 0 is used by the vertex shader (UBO for view transforms)
layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
   //outColor = texture(texSampler, fragTexCoord);
   outColor = textureLod(texSampler, fragTexCoord, 0); // unnormalized-coordinate texture samplers require SPIR-V opcode OpImageSampleExplicitLod
}