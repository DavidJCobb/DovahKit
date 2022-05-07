
#ifndef INCLUDE_GUARD_expand_light_and_shadow_inputs
#define INCLUDE_GUARD_expand_light_and_shadow_inputs

#include "structs/light_and_shadow_inputs.glsl"

#if !defined(SET_SCENE_STATE) || !defined(SET_ALL_LIGHTS)
   #error You included this file too early. Place it after the definition for the its index macro.
#endif
//
light_and_shadow_inputs expand_light_and_shadow_inputs(compact_light_and_shadow_inputs compact) {
   light_and_shadow_inputs expanded;
   expanded.pos_world           = compact.pos_world;
   expanded.sun_shadow_vert_pos = compact.sun_shadow_vert_pos;
   expanded.tangent_space       = compact.tangent_space;
   expanded.tangent_sun_dir     = compact.tangent_space * scene.sun_dir;
   expanded.tangent_vert_pos    = compact.tangent_space * compact.pos_world;
   expanded.light_space_pos     = compact.light_space_pos;
   for(int i = 0; i < 4; ++i) {
      expanded.vector_to_light[i]      = vec3(compact.vector_to_light[i]);
      expanded.light_distance_ratio[i] = compact.vector_to_light[i].w;
      //
      expanded.tangent_light_dir[i] = vec3(0, 0, 0);
      //
      int light_index = scene.shadow_caster_index[i];
      if (light_index >= 0) {
         expanded.tangent_light_dir[i] = normalize(compact.tangent_space * vec3(scene_lights[light_index].transform[3]));
      }
   }
   return expanded;
}

#endif