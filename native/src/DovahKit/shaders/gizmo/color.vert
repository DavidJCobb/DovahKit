#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : enable

#include "../includes/structs/scene_gizmo_state.glsl"
#include "../includes/structs/scene_global_state.glsl"

#define SET_SCENE_STATE    0
#define SET_GIZMO_STATE    1

#include "../includes/descriptor_sets/scene_state.glsl"
#include "../includes/descriptor_sets/gizmo_state.glsl"

DECLARE_SCENE_STATE_PARAMS
DECLARE_GIZMO_STATE_PARAMS

layout(location = 0) in vec4 in_position;

layout(location = 0) out vec4 out_color;

void main() {
   gl_Position = scene.proj * scene.view * gizmo.transform * vec4(in_position.xyz, 1.0);
   
   out_color = vec4(1, 1, 1, 1);

   if (in_position.w == 0) {
      out_color.rgb = gizmo.color_x;
   } else if (in_position.w == 1) {
      out_color.rgb = gizmo.color_y;
   } else if (in_position.w == 2) {
      out_color.rgb = gizmo.color_z;
   }

   // wouldn't it be great if we made a shader language that mimicked the aesthetics of C, but not for casting?
   // inconsistency is always very helpful to programmers
   int highlight_flag = 1 << 4 << int(in_position.w);

   if ((gizmo.flags & highlight_flag) == highlight_flag) {
      out_color.rgb = mix(out_color.rgb, gizmo.color_highlight.rgb, gizmo.color_highlight.a);
   }

   if ((gizmo.flags & SCENE_GIZMO_STATE_FLAG_MODE_ANY) == 0 ) {
      // hide off-screen to skip fragment rendering
      gl_Position = vec4(10, 10, 0, 1);
   }
}