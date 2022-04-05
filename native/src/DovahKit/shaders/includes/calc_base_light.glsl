
#ifndef included_calc_base_light // include guard
#define included_calc_base_light

#include "calc_specular_strength.glsl"

computed_light calc_base_light(
   vec3  tangent_light_dir,
   vec3  tangent_view_dir,
   vec3  normal,
   vec3  light_color,
   float specular_exponent,
   float attenuate
) {
   computed_light result;
   //
   float str_diff = max(dot(tangent_light_dir, normal), 0.0); // diffuse strength
   str_diff *= attenuate;
   //
   float str_spec = 0;
   if (str_diff > 0.0) {
      str_spec = calc_specular_strength(normal, tangent_light_dir, tangent_view_dir, specular_exponent) * attenuate;
   }
   //
   result.diffuse  = str_diff * light_color;
   result.specular = str_spec * light_color;
   return result;
}

#endif