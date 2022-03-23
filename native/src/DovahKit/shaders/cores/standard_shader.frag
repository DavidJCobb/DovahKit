
// #include this from a main shader, which defines outputs
//
// your shader's main() can return the result of calculate_color() verbatim
// or
// your shader can use it in other calculations (e.g. OIT)

#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : enable

#include "../includes/alpha_testing_conditional_discard.glsl"
#include "../includes/calc_specular_strength.glsl"
#include "../includes/calc_directional_shadow.glsl"
#include "../includes/computed_light.glsl"
#include "../includes/calc_directional_light.glsl"
#include "../includes/point_light.glsl"
#include "../includes/calc_point_light.glsl"

// configuration defines:
// USE_ALPHA_OIT == 0 or 1

layout (constant_id = 0) const int MAX_LIGHTS = 4;
#define SHADOW_CASTER_COUNT 4

#include "../includes/rendered_mesh_push_constant.glsl"
#include "../includes/rendered_mesh_shader_params.glsl"
#include "../includes/scene_global_state.glsl"

layout(std140,binding = 0) uniform UniformBufferObject {
   scene_global_state ubo;
};
layout(binding = 1) uniform sampler   default_sampler;
layout(binding = 2) uniform sampler2D sun_shadow_map;
layout(binding = 3) uniform sampler2D light_shadow_maps[SHADOW_CASTER_COUNT * 2];
layout(std140,set = 0, binding = 4) readonly buffer ObjectBuffer {
	rendered_mesh_shader_params objects[];
} objectBuffer;
layout(std140,set = 0, binding = 5) readonly buffer PointLightBuffer {
	point_light lights[MAX_LIGHTS];
} pointLightBuffer;
layout(binding = 6) uniform texture2D textures[];

#include "standard_shader/fragment_input.glsl"
layout(location = 0) in VS_OUT {
   fragment_input fs_in;
};

vec4 calculate_color() {
   vec4 color;
   //
   rendered_mesh_shader_params current_object = objectBuffer.objects[pushed.object_index];
   //
   color = texture(sampler2D(textures[pushed.texture_index], default_sampler), fs_in.uv);
   //
   color *= fs_in.color;
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
   // Apply all lights in the scene:
   //
   vec3 view_dir = normalize(fs_in.tangent_view_pos - fs_in.tangent_vert_pos); // direction from camera position to fragment position
   //
   computed_light light_data;
   if (pushed.receive_shadows == 0) {
      light_data = calc_directional_light(
         fs_in.tangent_sun_dir,
         ubo.sun_color,
         normal,
         view_dir,
         current_object.specular_exponent
      );
   } else {
      light_data = calc_directional_light_and_shadow(
         fs_in.tangent_sun_dir,
         fs_in.sun_shadow_vert_pos,
         ubo.sun_color,
         normal,
         view_dir,
         current_object.specular_exponent,
         sun_shadow_map
      );
   }
   for(int i = 0; i < MAX_LIGHTS; ++i) {
      computed_light current = calc_point_light(
         pointLightBuffer.lights[i],
         fs_in.tangent_space,
         normal,
         fs_in.tangent_vert_pos,
         view_dir,
         current_object.specular_exponent
      );
      if (point_light_can_cast_shadows(pointLightBuffer.lights[i])) { // if this light is allowed to cast shadows
         bool is_casting = false;
         mat4 light_proj;
         if (ubo.shadow_caster_index_0 == i) {
            is_casting = true;
            light_proj = ubo.shadow_caster_proj_0;
         } else if (ubo.shadow_caster_index_1 == i) {
            is_casting = true;
            light_proj = ubo.shadow_caster_proj_1;
         } else if (ubo.shadow_caster_index_2 == i) {
            is_casting = true;
            light_proj = ubo.shadow_caster_proj_2;
         } else if (ubo.shadow_caster_index_3 == i) {
            is_casting = true;
            light_proj = ubo.shadow_caster_proj_3;
         }
         if (is_casting) {
            //
            // TODO
            //
         }
      }
      light_data.diffuse  += current.diffuse;
      light_data.specular += current.specular;
   }
   light_data.specular *= current_object.specular_strength * current_object.specular_color;
   //
   color.rgb *= ubo.ambient_light_color + light_data.diffuse + light_data.specular;
   return color;
}