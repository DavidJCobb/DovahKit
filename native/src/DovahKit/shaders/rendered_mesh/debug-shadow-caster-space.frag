#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : enable

#include "../includes/alpha_testing_conditional_discard.glsl"

// configuration defines:
// USE_ALPHA_OIT == 0 or 1

layout (constant_id = 0) const int MAX_LIGHTS = 4;
layout (constant_id = 1) const int DEBUG_SHADOW_CASTER_INDEX = 0;

#include "../includes/point_light.glsl"
#include "../includes/rendered_mesh_push_constant.glsl"
#include "../includes/rendered_mesh_shader_params.glsl"
#include "../includes/scene_global_state.glsl"

layout(std140,binding = 0) uniform UniformBufferObject {
   scene_global_state ubo;
};
layout(binding = 1) uniform sampler default_sampler;
// binding 2: sun shadow map (not used in this shader)
// binding 3: light shadow maps (not used in this shader)
layout(std140,set = 0, binding = 4) readonly buffer ObjectBuffer {
	rendered_mesh_shader_params objects[];
} objectBuffer;
layout(std140,set = 0, binding = 5) readonly buffer PointLightBuffer {
	point_light lights[MAX_LIGHTS];
} pointLightBuffer;
layout(binding = 6) uniform texture2D textures[];

#include "../cores/standard_shader/fragment_input.glsl"
layout(location = 0) in VS_OUT {
   fragment_input fs_in;
};

vec4 calculate_color() {
   vec4 color;
   //
   rendered_mesh_shader_params current_object = objectBuffer.objects[pushed.object_index];
   //
   color    = texture(sampler2D(textures[pushed.texture_index], default_sampler), fs_in.uv);
   color.a *= fs_in.color.a;
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
   if (ubo.shadow_caster_index[DEBUG_SHADOW_CASTER_INDEX] < 0) {
      return vec4(1, 1, 1, 1);
   }
   vec3 which;
   {
      vec3 light_forward = vec3(pointLightBuffer.lights[ubo.shadow_caster_index[DEBUG_SHADOW_CASTER_INDEX]].transform[1]);
      vec3 light_pos     = vec3(pointLightBuffer.lights[ubo.shadow_caster_index[DEBUG_SHADOW_CASTER_INDEX]].transform[3]);
      vec3 distance      = fs_in.pos_world - light_pos;
      if (dot(normalize(light_forward), normalize(distance)) >= 0) { // vectors converging
         which = vec3(fs_in.light_shadow_vert_position_pos[DEBUG_SHADOW_CASTER_INDEX]);
      } else { // vectors diverging
         which = vec3(fs_in.light_shadow_vert_position_neg[DEBUG_SHADOW_CASTER_INDEX]);
      }
   }
   //
   color.r = which.x;
   color.g = which.y;
   color.b = which.z;
   return color;
}

layout(location = 0) out vec4 out_color;

void main() {
   out_color = calculate_color();
}