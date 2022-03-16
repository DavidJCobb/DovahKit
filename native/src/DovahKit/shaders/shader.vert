#version 450
#extension GL_EXT_nonuniform_qualifier : require

#define TBN_MODE_NONE     0
#define TBN_MODE_LAZY     1
#define TBN_MODE_THOROUGH 2
//
#define TBN_ORTHOGONALIZE_MODE TBN_MODE_NONE

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
   int   object_index;
	int   texture_index;
   int   texture_normal_index;
   float alpha_test_threshold;
   int   alpha_test_operation;
   int   enable_alpha_blending; // VkBool32
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
   vec3 sun_dir;
   vec3 sun_color;
	mat4 sun_space;
} ubo;
// binding 1 is used by the fragment shader (texture sampler)
// binding 2 is used by the fragment shader (shadow map)
layout(std430,set = 0, binding = 3) readonly buffer ObjectBuffer {
	ObjectData objects[];
} objectBuffer;
// binding 4 is used by the fragment shader (light array)
// binding 5 is used by the fragment shader (texture array)

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec4 in_color;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in vec3 in_normal;
layout(location = 4) in vec3 in_tangent;
layout(location = 5) in vec3 in_bitangent;

layout(location = 0) out VS_OUT {
   vec4  color;
   vec2  uv;
   vec3  pos_world;
   mat3  tangent_space;
   vec3  tangent_sun_dir;
   vec3  tangent_view_pos;
   vec3  tangent_vert_pos;
   vec4  sun_shadow_vert_pos;
   float camera_distance;
} vs_out;

// [-1, 1] to [0, 1]
const mat4 shadow_to_normalized_coords = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0
);

void main() {
   mat4 model_transform = objectBuffer.objects[pushed.object_index].transform;
   //
   gl_Position = ubo.proj * ubo.view * model_transform * vec4(in_position, 1.0);
   //
   vs_out.color     = in_color;
   vs_out.uv        = in_uv;
   vs_out.pos_world = vec3(model_transform * vec4(in_position, 1.0));
   vs_out.sun_shadow_vert_pos = (shadow_to_normalized_coords * ubo.sun_space * model_transform) * vec4(in_position, 1.0);
   //
   #if !defined(TBN_ORTHOGONALIZE_MODE) || TBN_ORTHOGONALIZE_MODE == TBN_MODE_NONE
      vs_out.tangent_space = transpose(mat3(
         normalize(vec3(model_transform * vec4(in_tangent,   0))), // multiplication by the model transform converts the TBN vectors to world-space
         normalize(vec3(model_transform * vec4(in_bitangent, 0))),
         normalize(vec3(model_transform * vec4(in_normal,    0)))  // NOTE: if non-uniform scaling is in use, then you must multiply by the transpose of the inverse of the rotation... but matrix inversions are slow on a GPU
      ));
   #else
      //
      // It's possible that we may receive tangent and bitangent vectors that aren't orthogonal 
      // with each other, or that aren't orthogonal with the normal vector. In these cases, we 
      // may need to "orthogonalize" them.
      //
      vec3 t;
      vec3 b;
      vec3 n;
      #if TBN_ORTHOGONALIZE_MODE == TBN_MODE_LAZY
         //
         // source: <https://stackoverflow.com/questions/62242055/how-are-normals-and-tangents-still-orthogonal-when-they-reach-the-pixel-fragment>
         //
         t = cross(in_bitangent, in_normal);
         b = cross(in_normal,    t);
         t = normalize(t);
         b = normalize(b);
         n = normalize(in_normal);
      #else
         #if TBN_ORTHOGONALIZE_MODE == TBN_MODE_THOROUGH
            //
            // Source: <https://stackoverflow.com/questions/46296323/normal-mapping-and-keep-the-tangents>
            //
            n  = normalize(in_normal);
            t  = in_tangent   - (dot(n, in_tangent)   * n);
            t /= dot(in_tangent,   t);
            b  = in_bitangent - (dot(n, in_bitangent) * n);
            b -= dot(b, t) * t;
            b /= dot(in_bitangent, b);
         #else
            #error Unrecognized TBN_ORTHOGONALIZE_MODE.
         #endif
      #endif
      //
      // Orthogonalization complete.
      //
      vs_out.tangent_space = transpose(mat3(
         normalize(vec3(model_transform * vec4(t, 0))), // multiplication by the model transform converts the TBN vectors to world-space
         normalize(vec3(model_transform * vec4(b, 0))),
         normalize(vec3(model_transform * vec4(n, 0)))  // NOTE: if non-uniform scaling is in use, then you must multiply by the transpose of the inverse of the rotation... but matrix inversions are slow on a GPU
      ));
   #endif
   vs_out.tangent_sun_dir  = vs_out.tangent_space * ubo.sun_dir;
   vs_out.tangent_view_pos = vs_out.tangent_space * vec3(ubo.view[3]);
   vs_out.tangent_vert_pos = vs_out.tangent_space * vs_out.pos_world;
   //
   vs_out.camera_distance = (ubo.view * vec4(vs_out.pos_world, 1.0)).z;
}