#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : enable

#define TBN_MODE_NONE     0
#define TBN_MODE_LAZY     1
#define TBN_MODE_THOROUGH 2
//
#define TBN_ORTHOGONALIZE_MODE TBN_MODE_NONE

layout (constant_id = 0) const int MAX_LIGHTS = 4;

#include "includes/rendered_light_shader_params.glsl"
#include "includes/rendered_mesh_push_constant.glsl"
#include "includes/rendered_mesh_shader_params.glsl"
#include "includes/scene_global_state.glsl"

layout(std430,binding = 0) uniform UniformBufferObject {
   scene_global_state scene;
};
// binding 1 is used by the fragment shader (texture sampler)
// binding 2 is used by the fragment shader (sun shadow map)
// binding 3 is used by the fragment shader (light shadow maps)
layout(std430,set = 0, binding = 4) readonly buffer ObjectBuffer {
	rendered_mesh_shader_params scene_meshes[];
};
layout(std430,set = 0, binding = 5) readonly buffer PointLightBuffer {
	rendered_light_shader_params scene_lights[MAX_LIGHTS];
};
// binding 6 is used by the fragment shader (texture array)

#include "includes/standard_vertex_inputs.glsl"

#include "cores/standard_shader/fragment_input.glsl"
layout(location = 0) out VS_OUT {
   fragment_input vs_out;
};

// [-1, 1] to [0, 1]
const mat4 shadow_to_normalized_coords = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0
);

void main() {
   mat4 model_transform = scene_meshes[pushed.object_index].transform;
   //
   gl_Position = scene.proj * scene.view * model_transform * vec4(in_position, 1.0);
   //
   vs_out.color     = in_color;
   vs_out.uv        = in_uv;
   vs_out.pos_world = vec3(model_transform * vec4(in_position, 1.0));
   vs_out.sun_shadow_vert_pos = (shadow_to_normalized_coords * scene.sun_space * model_transform) * vec4(in_position, 1.0);
   //
   for(int i = 0; i < 4; ++i) {
      vs_out.vector_to_light[i] = vec3(0, 0, 0);
      vs_out.light_space_pos[i] = vs_out.pos_world;
      //
      int light_index = scene.shadow_caster_index[i];
      if (light_index >= 0) {
         vs_out.vector_to_light[i]      = vs_out.pos_world - vec3(scene_lights[light_index].transform[3]);
         vs_out.light_distance_ratio[i] = length(vs_out.vector_to_light[i]) / scene_lights[light_index].radius;
         vs_out.light_space_pos[i]      = vec3(scene_lights[i].transform_inv * vec4(vs_out.pos_world, 1.0));
      }
   }
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
   vs_out.tangent_sun_dir  = vs_out.tangent_space * scene.sun_dir;
   vs_out.tangent_view_pos = vs_out.tangent_space * vec3(scene.view[3]);
   vs_out.tangent_vert_pos = vs_out.tangent_space * vs_out.pos_world;
   for(int i = 0; i < 4; ++i) {
      vs_out.tangent_light_dir[i] = vec3(0, 0, 0);
      //
      int light_index = scene.shadow_caster_index[i];
      if (light_index >= 0) {
         vs_out.tangent_light_dir[i] = normalize(vs_out.tangent_space * vec3(scene_lights[light_index].transform[3]));
      }
   }
   //
   vs_out.camera_distance = (scene.view * vec4(vs_out.pos_world, 1.0)).z;
}