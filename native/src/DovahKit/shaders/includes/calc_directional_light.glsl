
// you must include the following files from your shader
// we can't include them here because GLSLC (GLSL to SPIR-V) requires a Google extension 
// for includes, and Google apparently didn't see any point in implementing #pragma once
//
//  - calc_directional_shadow.glsl
//  - computed_light.glsl

// inputs are in tangent space, where applicable
computed_light calc_directional_light(
   vec3  light_dir,       // tangent-space light direction
   vec4  light_space_pos, // light-space vertex position
   vec3  light_color,
   vec3  normal,          // surface normal
   vec3  view_dir,        // tangent-space view direction
   float specular_exponent,
   sampler2D shadow_map
) {
   computed_light result;
   //
   light_dir = -normalize(light_dir);
   //
   float str_diff = max(dot(normal, light_dir) - calc_directional_shadow(normal, light_dir, light_space_pos, shadow_map), 0.0);
   float str_spec = calc_specular_strength(normal, light_dir, view_dir, specular_exponent);
   //
   result.diffuse  = str_diff * light_color;
   result.specular = str_spec * light_color;
   return result;
}