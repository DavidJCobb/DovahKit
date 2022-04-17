
// #include this from a main shader, which defines outputs
//
// your shader's main() can return the result of calculate_color() verbatim
// or
// your shader can use it in other calculations (e.g. OIT)

#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : enable

#define PRE_CHECK_AND_SKIP_SHADOW_CASTER_CALCS 0
#define EMULATE_OMNI_LIGHT_SHADOW_SEAM 1

#include "../includes/rendered_light_shader_params.glsl"
#include "../includes/rendered_mesh_shader_params.glsl"
#include "../includes/scene_global_state.glsl"

#include "../includes/alpha_testing_conditional_discard.glsl"
#include "../includes/calc_specular_strength.glsl"
#include "../includes/calc_directional_shadow.glsl"
#include "../includes/computed_light.glsl"
#include "../includes/calc_directional_light.glsl"
#include "../includes/calc_point_light.glsl"
#include "../includes/calc_point_shadow.glsl"
#include "../includes/calc_spot_light.glsl"

// configuration defines:
// USE_ALPHA_OIT == 0 or 1

layout (constant_id = 0) const int MAX_LIGHTS = 4;
#define SHADOW_CASTER_COUNT 4

#include "../includes/rendered_mesh_push_constant.glsl"

layout(std430,binding = 0) uniform UniformBufferObject {
   scene_global_state scene;
};
layout(binding = 1) uniform sampler     default_sampler;
layout(binding = 2) uniform sampler2D   sun_shadow_map;
layout(binding = 3) uniform samplerCube light_shadow_maps[SHADOW_CASTER_COUNT];
layout(std140,set = 0, binding = 4) readonly buffer ObjectBuffer {
	rendered_mesh_shader_params scene_meshes[];
};
layout(std430,set = 0, binding = 5) readonly buffer PointLightBuffer {
	rendered_light_shader_params scene_lights[MAX_LIGHTS];
};
layout(binding = 6) uniform texture2D textures[];

#include "standard_shader/fragment_input.glsl"
layout(location = 0) in VS_OUT {
   fragment_input fs_in;
};

const mat4 shadow_to_normalized_coords = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0
);

vec4 calculate_color() {
   if (scene.interior_clip_distance > 0) {
      if (fs_in.camera_distance > scene.interior_clip_distance)
         discard;
   }
   //
   vec4 color;
   //
   rendered_mesh_shader_params current_object = scene_meshes[pushed.object_index];
   //
   color = texture(sampler2D(textures[pushed.texture_index], default_sampler), fs_in.uv);
   //
   color *= fs_in.color;
   alpha_testing_conditional_discard(color.a, pushed.alpha_test_operation, pushed.alpha_test_threshold);
   #if USE_ALPHA_OIT == 1
      if (pushed.enable_alpha_blending == 0) {
         //
         // If alpha blending is disabled, ignore the vertex alpha and the 
         // texture alpha.
         //
         color.a = 1.0;
      }
   #else
      color.a = 1.0;
   #endif
   //
   vec3 normal = vec3(0, 0, 1);
   if (pushed.texture_normal_index >= 0) {
      normal = texture(sampler2D(textures[pushed.texture_normal_index], default_sampler), fs_in.uv).rgb;
      normal = normalize(normal * 2.0 - 1.0);
   }
   //
   // Apply all lights in the scene:
   //
   vec3 tangent_view_dir = normalize(fs_in.tangent_view_pos - fs_in.tangent_vert_pos); // direction from camera position to fragment position
   //
   computed_light light_data;
   if (pushed.receive_shadows == 0) {
      light_data = calc_directional_light(
         fs_in.tangent_sun_dir,
         scene.sun_color,
         normal,
         tangent_view_dir,
         current_object.specular_exponent
      );
   } else {
      light_data = calc_directional_light_and_shadow(
         fs_in.tangent_sun_dir,
         fs_in.sun_shadow_vert_pos,
         scene.sun_color,
         normal,
         tangent_view_dir,
         current_object.specular_exponent,
         sun_shadow_map
      );
   }
   //
   // Point lights
   //
   {
      #if PRE_CHECK_AND_SKIP_SHADOW_CASTER_CALCS
      bool any_casters = false;
      for(int i = 0; i < SHADOW_CASTER_COUNT; ++i) {
         if (scene.shadow_caster_index[i] < 0)
            continue;
         if (fs_in.light_distance_ratio[i] > 1)
            continue;
         any_casters = true;
         break;
      }
      if (any_casters) {
      #endif
         for(int i = 0; i < MAX_LIGHTS; ++i) {
            int light_type = scene_lights[i].type;
            //
            computed_light current;
            if (light_type == RENDERED_LIGHT_TYPE_SPOT_SHADOW) {
               current = calc_spot_light(
                  scene_lights[i],
                  fs_in.tangent_space,
                  normal,
                  fs_in.tangent_vert_pos,
                  tangent_view_dir,
                  current_object.specular_exponent
               );
            } else {
               current = calc_point_light(
                  scene_lights[i],
                  fs_in.tangent_space,
                  normal,
                  fs_in.tangent_vert_pos,
                  tangent_view_dir,
                  current_object.specular_exponent
               );
            }
            //
            float shadow = 0.0;
            if (rendered_light_can_cast_shadows(scene_lights[i])) { // if this light is allowed to cast shadows
               for(int j = 0; j < SHADOW_CASTER_COUNT; ++j) {
                  if (scene.shadow_caster_index[j] == i) {
                     {
                        vec3  light_direction = normalize(fs_in.light_space_pos[j]);
                        float yaw_offset      = atan(light_direction.y, light_direction.x);
                        int   light_type      = scene_lights[i].type;
                        //
                        if (light_type == RENDERED_LIGHT_TYPE_OMNI_SHADOW) {
                           //
                           // Omni-shadow lights cast light and shadows in all directions. However, due to Bethesda's 
                           // approach to rendering them, there is a seam in the shadow no wider than one degree. The 
                           // seam follows the light's local YZ plane, i.e. it is a ring that reaches forward, back, 
                           // up, and down (all light-relative directions).
                           //
                           #if EMULATE_OMNI_LIGHT_SHADOW_SEAM
                              //
                              // Bethesda doesn't use cubemap shadows; to reduce VRAM usage, they use two 179-degree-FOV 
                              // shadow maps stitched together. (GPUs use rectilinear projection; 180-degree FOVs and 
                              // above are mathematically impossible.) This results in a seam -- a gap where there are 
                              // no shadows.
                              //
                              if (abs(abs(yaw_offset) - radians(90)) < radians(1)) { // within one degree of (+/-)90deg
                                 break;
                              }
                           #endif
                        } else if (light_type == RENDERED_LIGHT_TYPE_HEMI_SHADOW) {
                           //
                           // Hemi lights cast light in all directions, but cast shadows only over a 179-degree range 
                           // on the local +X side, spanning from local -Y to local +Y.
                           //
                           if (abs(yaw_offset) > radians(89)) {
                              break;
                           }
                        } else if (light_type == RENDERED_LIGHT_TYPE_SPOT_SHADOW) {
                           //
                           // TODO
                           //
                        }
                     }
                     //
                     // Compute shadows:
                     //
                     shadow = calc_point_shadow(
                        normal,
                        fs_in.tangent_light_dir[j],
                        fs_in.light_distance_ratio[j],
                        fs_in.vector_to_light[j],
                        light_shadow_maps[j]
                     );
                     break;
                  }
               }
            }
            shadow = 1.0 - shadow;
            //
            light_data.diffuse  += current.diffuse  * shadow;
            light_data.specular += current.specular * shadow;
         }
      #if PRE_CHECK_AND_SKIP_SHADOW_CASTER_CALCS
      } else {
         for(int i = 0; i < MAX_LIGHTS; ++i) {
            int light_type = scene_lights[i].type;
            //
            computed_light current;
            if (light_type == RENDERED_LIGHT_TYPE_SPOT_SHADOW) {
               current = calc_spot_light(
                  scene_lights[i],
                  fs_in.tangent_space,
                  normal,
                  fs_in.tangent_vert_pos,
                  tangent_view_dir,
                  current_object.specular_exponent
               );
            } else {
               current = calc_point_light(
                  scene_lights[i],
                  fs_in.tangent_space,
                  normal,
                  fs_in.tangent_vert_pos,
                  tangent_view_dir,
                  current_object.specular_exponent
               );
            }
      }
      #endif
   }
   //
   // Lights and shadows done.
   //
   light_data.specular *= current_object.specular_strength * current_object.specular_color;
   //
   color.rgb *= scene.ambient_light_color + light_data.diffuse + light_data.specular;
   //
   // Fog:
   //
   {
      float range = scene.fog_plane_far - scene.fog_plane_near;
      float coord = clamp((fs_in.camera_distance - scene.fog_plane_near) / range, 0.0F, 1.0F);
      float visibility = pow(coord, clamp(scene.fog_power, 0.0F, 1.0F));
      visibility = max(1.0F - min(1.0F, scene.fog_max), visibility);
      //
      vec3 fog_color = (scene.fog_color_far * (1.0F - visibility)) + (scene.fog_color_near * (visibility));
      color.rgb = (fog_color * (1.0F - visibility)) + (color.rgb * visibility);
   }
   //
   return color;
}