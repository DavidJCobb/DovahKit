
#ifndef INCLUDE_GUARD_calc_directional_light
#define INCLUDE_GUARD_calc_directional_light

#include "calc_directional_shadow.glsl"
#include "calc_specular_strength.glsl"
#include "structs/computed_light.glsl"

// inputs are in tangent space, where applicable
computed_light calc_directional_light(
   vec3  light_dir,       // tangent-space light direction
   vec3  light_color,
   vec3  normal,          // surface normal
   vec3  view_dir,        // tangent-space view direction
   float specular_exponent
) {
   computed_light result;
   //
   light_dir = -normalize(light_dir);
   //
   float str_diff = max(dot(normal, light_dir), 0.0);
   float str_spec = calc_specular_strength(normal, light_dir, view_dir, specular_exponent);
   //
   result.diffuse  = str_diff * light_color;
   result.specular = str_spec * light_color;
   return result;
}

computed_light calc_directional_light_and_shadow(
   vec3  light_dir,       // tangent-space light direction
   vec4  light_space_pos, // light-space vertex position
   vec3  light_color,
   vec3  normal,          // surface normal
   vec3  normal_geo_only, // surface normal NOT including normal map
   vec3  view_dir,        // tangent-space view direction
   float specular_exponent,
   sampler2D shadow_map
) {
   computed_light result;
   //
   light_dir = -normalize(light_dir);
   //
   float shadow   = calc_directional_shadow(normal, normal_geo_only, light_dir, light_space_pos, shadow_map);
   float str_diff = max(dot(normal, light_dir) - shadow, 0.0);
   float str_spec = max(calc_specular_strength(normal, light_dir, view_dir, specular_exponent) - shadow, 0.0);
   //
   result.diffuse  = str_diff * light_color;
   result.specular = str_spec * light_color;
   return result;
}

#endif