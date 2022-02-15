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
   int texture_normal_index;
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

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_color;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in vec3 in_normal;
layout(location = 4) in vec3 in_tangent;
layout(location = 5) in vec3 in_bitangent;

layout(location = 0) out VS_OUT {
   vec3 color;
   vec2 uv;
   vec3 pos_world;
   mat3 tangent_space;
   vec3 tangent_sun_pos;
   vec3 tangent_view_pos;
   vec3 tangent_vert_pos;
} vs_out;

void main() {
   mat4 model_transform = objectBuffer.objects[pushed.object_index].transform;
   //
   gl_Position = ubo.proj * ubo.view * model_transform * vec4(in_position, 1.0);
   //
   vs_out.color     = in_color;
   vs_out.uv        = in_uv;
   vs_out.pos_world = vec3(model_transform * vec4(in_position, 1.0));
   //
   vs_out.tangent_space = transpose(mat3(
      normalize(vec3(model_transform * vec4(in_tangent,   0))),
      normalize(vec3(model_transform * vec4(in_bitangent, 0))),
      normalize(vec3(model_transform * vec4(in_normal,    0))) // NOTE: if non-uniform scaling is in use, then you must multiply by the transpose of the inverse of the rotation... but matrix inversions are slow on a GPU
   ));
   vs_out.tangent_sun_pos  = vs_out.tangent_space * ubo.sun_pos;
   vs_out.tangent_view_pos = vs_out.tangent_space * vec3(ubo.view[3]);
   vs_out.tangent_vert_pos = vs_out.tangent_space * vec3(model_transform * vec4(in_position, 1.0));
}