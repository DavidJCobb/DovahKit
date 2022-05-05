
#if !defined(SET_SCENE_STATE) || !defined(SET_SHADOW_MAPS)
   #error You included this file too early. Place it after the definition for the its index macro.
#endif

//
// Including file MUST...
//
//  - Define a value called MAX_LIGHTS somewhere, ideally as a specialization constant
//
//  - Define a value called SHADOW_CASTER_COUNT somewhere, ideally as a specialization constant
//
//  - Include and declare the "scene state" and "shadow maps" descriptor sets
//

#include "structs/computed_light.glsl"
#include "structs/light_and_shadow_inputs.glsl"
#include "structs/rendered_light_shader_params.glsl"
#include "structs/scene_global_state.glsl"
#include "calc_directional_shadow.glsl"
#include "calc_directional_light.glsl"
#include "calc_point_light.glsl"
#include "calc_point_shadow.glsl"
#include "calc_spot_light.glsl"

computed_light calc_all_light_and_shadow(
   light_and_shadow_inputs light_inputs,
   //
   int   receive_shadows,
   vec3  specular_color,
   float specular_exponent,
   float specular_strength,
   vec3  texture_normal
) {
   vec3 tangent_view_dir = normalize(light_inputs.tangent_view_pos - light_inputs.tangent_vert_pos); // direction from camera position to fragment position
   //
   computed_light light_data;
   if (receive_shadows == 0) {
      light_data = calc_directional_light(
         light_inputs.tangent_sun_dir,
         scene.sun_color,
         texture_normal,
         tangent_view_dir,
         specular_exponent
      );
   } else {
      light_data = calc_directional_light_and_shadow(
         light_inputs.tangent_sun_dir,
         light_inputs.sun_shadow_vert_pos,
         scene.sun_color,
         texture_normal,
         tangent_view_dir,
         specular_exponent,
         sun_shadow_map
      );
   }
   //
   // Point lights
   //
   {
      #if defined(PRE_CHECK_AND_SKIP_SHADOW_CASTER_CALCS)
      #if PRE_CHECK_AND_SKIP_SHADOW_CASTER_CALCS != 0
      bool any_casters = false;
      for(int i = 0; i < SHADOW_CASTER_COUNT; ++i) {
         if (scene.shadow_caster_index[i] < 0)
            continue;
         if (light_inputs.light_distance_ratio[i] > 1)
            continue;
         any_casters = true;
         break;
      }
      if (any_casters) {
      #endif
      #endif
         for(int i = 0; i < MAX_LIGHTS; ++i) {
            int light_type = scene_lights[i].type;
            //
            computed_light current;
            if (light_type == RENDERED_LIGHT_TYPE_SPOT_SHADOW) {
               current = calc_spot_light(
                  scene_lights[i],
                  light_inputs.tangent_space,
                  texture_normal,
                  light_inputs.tangent_vert_pos,
                  tangent_view_dir,
                  specular_exponent
               );
            } else {
               current = calc_point_light(
                  scene_lights[i],
                  light_inputs.tangent_space,
                  texture_normal,
                  light_inputs.tangent_vert_pos,
                  tangent_view_dir,
                  specular_exponent
               );
            }
            //
            float shadow = 0.0;
            if (rendered_light_can_cast_shadows(scene_lights[i])) { // if this light is allowed to cast shadows
               for(int j = 0; j < SHADOW_CASTER_COUNT; ++j) {
                  if (scene.shadow_caster_index[j] == i) {
                     {
                        vec3  light_direction = normalize(light_inputs.light_space_pos[j]);
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
                        texture_normal,
                        light_inputs.tangent_light_dir[j],
                        light_inputs.light_distance_ratio[j],
                        light_inputs.vector_to_light[j],
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
      #if defined(PRE_CHECK_AND_SKIP_SHADOW_CASTER_CALCS)
      #if PRE_CHECK_AND_SKIP_SHADOW_CASTER_CALCS != 0
      } else {
         for(int i = 0; i < MAX_LIGHTS; ++i) {
            int light_type = scene_lights[i].type;
            //
            computed_light current;
            if (light_type == RENDERED_LIGHT_TYPE_SPOT_SHADOW) {
               current = calc_spot_light(
                  scene_lights[i],
                  light_inputs.tangent_space,
                  texture_normal,
                  light_inputs.tangent_vert_pos,
                  tangent_view_dir,
                  specular_exponent
               );
            } else {
               current = calc_point_light(
                  scene_lights[i],
                  light_inputs.tangent_space,
                  texture_normal,
                  light_inputs.tangent_vert_pos,
                  tangent_view_dir,
                  specular_exponent
               );
            }
      }
      #endif
      #endif
   }
   //
   // Lights and shadows done.
   //
   light_data.specular *= specular_strength * specular_color;
   return light_data;
}