
#ifndef INCLUDE_GUARD_calc_spot_light
#define INCLUDE_GUARD_calc_spot_light

#include "calc_base_light.glsl"
#include "structs/computed_light.glsl"
#include "structs/rendered_light_shader_params.glsl"

computed_light calc_spot_light(
   rendered_light_shader_params light,
   mat3  tangent_space,
   vec3  normal,           // surface normal
   vec3  tangent_vert_pos,
   vec3  tangent_view_dir,
   float specular_exponent
) {
   vec3  tangent_light_pos = tangent_space * vec3(light.transform[3]);
   vec3  tangent_light_dir = tangent_light_pos - tangent_vert_pos;
   float distance = length(tangent_light_dir);
   tangent_light_dir = normalize(tangent_light_dir);
   //
   // Spotlights shine out to (radius + 16) units away from the light position.
   //
   if (distance > light.radius + 16) {
      computed_light no_op;
      no_op.diffuse  = vec3(0, 0, 0);
      no_op.specular = vec3(0, 0, 0);
      return no_op;
   }
   float attenuate = clamp(1.0 - distance / (light.radius + 16), 0.0, 1.0);
   attenuate *= attenuate;
   //
   float intensity = 1.0 - clamp(
      acos(
         dot(
            normalize(tangent_space * vec3(light.transform[0])), // light-relative +X to tangent space
            -tangent_light_dir
         )
      ) / (light.fov / 2),
      0.0,
      1.0
   );
   intensity = pow(intensity, light.falloff);
   attenuate *= intensity;
   //
   computed_light result = calc_base_light(
      tangent_light_dir,
      tangent_view_dir,
      normal,
      light.color,
      specular_exponent,
      attenuate
   );
   return result;
}

#endif