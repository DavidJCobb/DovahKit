#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(push_constant) uniform PER_OBJECT {
   int object_index;
	int texture_index;
} pushed;

// binding 0 is used by the vertex shader (UBO for camera/view transforms)
layout(binding = 1) uniform sampler   texSampler;
// binding 2 is used by the vertex shader (buffer for object data)
layout(binding = 3) uniform texture2D textures[];

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
   //outColor = vec4(fragTexCoord, 0.0, 1.0);
   outColor  = texture(sampler2D(textures[pushed.texture_index], texSampler), fragTexCoord);
   outColor *= vec4(fragColor, 1.0);
}