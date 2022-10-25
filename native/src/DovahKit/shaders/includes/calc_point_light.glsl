
#ifndef INCLUDE_GUARD_calc_point_light
#define INCLUDE_GUARD_calc_point_light

#include "calc_base_light.glsl"
#include "structs/computed_light.glsl"
#include "structs/rendered_light_shader_params.glsl"

// inputs except (light) are in tangent space, where applicable
computed_light calc_point_light(
   rendered_light_shader_params light,
   mat3  tangent_space,
   vec3  normal,           // surface normal
   vec3  tangent_vert_pos,
   vec3  tangent_view_dir,
   float specular_exponent
) {
   vec3  tangent_light_pos = tangent_space * vec3(light.transform[3]);
   vec3  tangent_light_dir = tangent_light_pos - tangent_vert_pos;
   float distance          = length(tangent_light_dir);
   float attenuate         = 1.0 - smoothstep(0.0, light.radius, distance);
   tangent_light_dir = normalize(tangent_light_dir);
   if (distance > light.radius) {
      computed_light no_op;
      no_op.diffuse  = vec3(0, 0, 0);
      no_op.specular = vec3(0, 0, 0);
      return no_op;
   }
   return calc_base_light(
      tangent_light_dir,
      tangent_view_dir,
      normal,
      light.color,
      specular_exponent,
      attenuate
   );
}

#endif