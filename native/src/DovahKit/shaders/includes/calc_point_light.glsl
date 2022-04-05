
#include "calc_specular_strength.glsl"
#include "computed_light.glsl"
#include "rendered_light_shader_params.glsl"

// inputs except (light) are in tangent space, where applicable
computed_light calc_point_light(
   rendered_light_shader_params light,
   mat3  tangent_space,
   vec3  normal,          // surface normal
   vec3  light_space_pos, // light-space vertex position
   vec3  view_dir,        // tangent-space view direction
   float specular_exponent
) {
   computed_light result;
   //
   vec3  light_pos = tangent_space * vec3(light.transform[3]);
   vec3  light_dir = normalize(light_pos - light_space_pos);
   float str_diff  = max(dot(light_dir, normal), 0.0); // diffuse strength
   //
   float distance  = length(light_pos - light_space_pos);
   float attenuate = 1.0 - smoothstep(0.0, light.radius, distance);
   //
   str_diff *= attenuate;
   //
   float str_spec = 0;
   if (str_diff > 0.0) {
      str_spec = calc_specular_strength(normal, light_dir, view_dir, specular_exponent) * attenuate;
   }
   //
   result.diffuse  = str_diff * light.color;
   result.specular = str_spec * light.color;
   return result;
}