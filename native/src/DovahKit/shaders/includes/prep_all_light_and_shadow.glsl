
#if !defined(SET_SCENE_STATE) || !defined(SET_ALL_LIGHTS)
   #error You included this file too early. Place it after the definition for the its index macro.
#endif

#include "structs/light_and_shadow_inputs.glsl"

// For use in a vertex shader.

compact_light_and_shadow_inputs prep_all_light_and_shadow(
   vec3 pos_world,
   mat3 tangent_space
) {
   compact_light_and_shadow_inputs result;
   //
   result.pos_world = pos_world;
   {
      const mat4 shadow_to_normalized_coords = mat4(  // [-1, 1] to [0, 1] for X and Y only
	      0.5, 0.0, 0.0, 0.0,
	      0.0, 0.5, 0.0, 0.0,
	      0.0, 0.0, 1.0, 0.0,
	      0.5, 0.5, 0.0, 1.0
      );
      result.sun_shadow_vert_pos = (shadow_to_normalized_coords * scene.sun_space) * vec4(pos_world, 1.0);
   }
   for(int i = 0; i < 4; ++i) {
      result.vector_to_light[i] = vec4(0, 0, 0, 2);
      result.light_space_pos[i] = pos_world;
      //
      int light_index = scene.shadow_caster_index[i];
      if (light_index >= 0) {
         vec3  vector_to_light      = pos_world - vec3(scene_lights[light_index].transform[3]);
         float light_distance_ratio = length(result.vector_to_light[i]) / scene_lights[light_index].radius;
         //
         result.vector_to_light[i] = vec4(vector_to_light, light_distance_ratio);
         result.light_space_pos[i] = vec3(scene_lights[i].transform_inv * vec4(pos_world, 1.0));
      }
   }
   //
   result.tangent_space    = tangent_space;
   result.tangent_view_pos = tangent_space * vec3(scene.view[3]);
   //
   return result;
}