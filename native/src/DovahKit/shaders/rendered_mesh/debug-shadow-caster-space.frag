#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : enable

#include "../includes/alpha_testing_conditional_discard.glsl"

// configuration defines:
// USE_ALPHA_OIT == 0 or 1

layout (constant_id = 0) const int MAX_LIGHTS = 4;
layout (constant_id = 1) const int DEBUG_SHADOW_CASTER_INDEX = 0;

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
// binding 5: point light list (not used in this shader)
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
   vec4  light_space_pos_pos = fs_in.light_shadow_vert_position_pos[DEBUG_SHADOW_CASTER_INDEX];
   vec4  light_space_pos_neg = fs_in.light_shadow_vert_position_neg[DEBUG_SHADOW_CASTER_INDEX];
   light_space_pos_pos /= light_space_pos_pos.w;
   light_space_pos_neg /= light_space_pos_neg.w;
   float x;
   float y;
   float z;
   {
      float a = abs(light_space_pos_pos.z);
      float b = abs(light_space_pos_neg.z);
      if (a < b) {
         z = a;
         x = light_space_pos_pos.x;
         y = light_space_pos_pos.y;
      } else {
         z = b;
         x = light_space_pos_neg.x;
         y = light_space_pos_neg.y;
      }
   }
   //
   color.r = (z + x) * 0.5;
   color.g = (z + y) * 0.5;
   color.b = z;
   return color;
}

layout(location = 0) out vec4 out_color;

void main() {
   out_color = calculate_color();
}