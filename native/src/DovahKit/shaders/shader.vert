#version 450

//
// Location sizes:
//
//  - most types | 1
//  - double     | 1
//  - dvec2      | 1
//  - dvec3      | 2
//  - dvec4      | 2
//
// Care must be taken to ensure that parameters don't overlap... unless 
// they're meant to.
//

layout(push_constant) uniform PER_OBJECT {
	int texture_index;
   int object_index;
} pushed;

struct ObjectData{
	mat4 transform;
};

layout(binding = 0) uniform UniformBufferObject {
   mat4 view;
   mat4 proj;
} ubo;
// binding 1 is used by the fragment shader (texture sampler)
// binding 2 is used by the fragment shader (texture array)
layout(std140,set = 0, binding = 3) readonly buffer ObjectBuffer {
	ObjectData objects[];
} objectBuffer;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
   mat4 model_transform = objectBuffer.objects[pushed.object_index].transform;
   //
   gl_Position  = ubo.proj * ubo.view * model_transform * vec4(inPosition, 1.0);
   fragColor    = inColor;
   fragTexCoord = inTexCoord;
}