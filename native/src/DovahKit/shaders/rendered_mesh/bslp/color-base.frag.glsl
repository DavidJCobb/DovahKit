
// #include this from a main shader, which defines outputs
//
// your shader's main() can return the result of calculate_color() verbatim
// or
// your shader can use it in other calculations (e.g. OIT)

#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : enable

#define PRE_CHECK_AND_SKIP_SHADOW_CASTER_CALCS 0
#define EMULATE_OMNI_LIGHT_SHADOW_SEAM 1

#include "../../includes/structs/rendered_light_shader_params.glsl"
#include "../../includes/structs/rendered_mesh_shader_params.glsl"
#include "../../includes/structs/scene_global_state.glsl"

#include "../../includes/alpha_testing_conditional_discard.glsl"
#include "../../includes/structs/computed_light.glsl"

// configuration defines:
// USE_ALPHA_OIT == 0 or 1

layout (constant_id = 0) const int MAX_LIGHTS = 4;
#define SHADOW_CASTER_COUNT 4

#include "../../includes/structs/rendered_mesh_push_constant.glsl"

#include "color.descriptors.glsl"
DECLARE_SCENE_STATE_PARAMS
DECLARE_ALL_TEXTURES_PARAMS
DECLARE_ALL_MESHES_PARAMS
DECLARE_ALL_LIGHTS_PARAMS
DECLARE_SHADOW_MAPS_PARAMS

#include "color.fragment-input.glsl"
layout(location = 0) in VS_OUT {
   fragment_input fs_in;
};

#include "../../includes/apply_scene_fog.glsl"
#include "../../includes/calc_all_light_and_shadow.glsl"
#include "../../includes/expand_light_and_shadow_inputs.glsl"

vec4 calculate_color() {
   float camera_distance = distance(fs_in.lighting_data.pos_world, scene.camera_pos);
   if (scene.interior_clip_distance > 0) {
      if (camera_distance > scene.interior_clip_distance)
         discard;
   }
   //
   vec4 color;
   //
   rendered_mesh_shader_params current_object = scene_meshes[pushed.object_index];
   //
   if (pushed.texture_index >= 0) {
      color = texture(sampler2D(textures[pushed.texture_index], default_sampler), fs_in.uv);
      color *= fs_in.color;
   } else {
      color = fs_in.color;
   }
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
   {
      computed_light light_data = calc_all_light_and_shadow(
         expand_light_and_shadow_inputs(fs_in.lighting_data),
         pushed.receive_shadows,
         current_object.specular_color,
         current_object.specular_exponent,
         current_object.specular_strength,
         normal
      );
      color.rgb *= scene.ambient_light_color + light_data.diffuse + light_data.specular;
   }
   //
   apply_scene_fog(color, camera_distance);
   //
   return color;
}