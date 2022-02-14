#version 450
#extension GL_EXT_nonuniform_qualifier : require

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
   int object_index;
	int texture_index;
} pushed;

struct ObjectData{
	mat4  transform;
   vec3  specular_color;
   float specular_strength;
   float specular_exponent;
};

layout(std140,binding = 0) uniform UniformBufferObject {
   mat4 view;
   mat4 proj;
   vec3 ambient_light_color;
   vec3 sun_pos;
   vec3 sun_color;
} ubo;
// binding 1 is used by the fragment shader (texture sampler)
layout(std140,set = 0, binding = 2) readonly buffer ObjectBuffer {
	ObjectData objects[];
} objectBuffer;
// binding 3 is used by the fragment shader (texture array)

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out VS_OUT {
   vec3 color;
   vec2 uv;
   vec3 normal;
   vec3 pos_world;
} vs_out;

void main() {
   mat4 model_transform = objectBuffer.objects[pushed.object_index].transform;
   //
   gl_Position = ubo.proj * ubo.view * model_transform * vec4(inPosition, 1.0);
   //
   vs_out.color     = inColor;
   vs_out.uv        = inTexCoord;
   vs_out.normal    = mat3(model_transform) * inNormal; // NOTE: if non-uniform scaling is in use, then you must multiply by the transpose of the inverse of the rotation... but matrix inversions are slow on a GPU
   vs_out.pos_world = vec3(model_transform * vec4(inPosition, 1.0));
}