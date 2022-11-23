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

//
// in_position.w is formatted as follows:
//
//  - Low two bits indicate the axis (X/Y/Z) and are used for coloration
//  - Next two bits indicate the gizmo model
//

void main() {
   int metadata = int(in_position.w);
   {
      int vertex_model  = metadata >> 2;
      int desired_model = (gizmo.flags & SCENE_GIZMO_STATE_FLAG_BITS_MODE);
      if (vertex_model != desired_model) {
         gl_Position = vec4(99, 99, 0, 0);
         return;
      }
   }

   gl_Position = scene.proj * scene.view * gizmo.transform * vec4(in_position.xyz, 1.0);
   
   out_color = vec4(1, 1, 1, 1);

   int axis = metadata & 3;
   if (axis == 0) {
      out_color.rgb = gizmo.color_x;
   } else if (axis == 1) {
      out_color.rgb = gizmo.color_y;
   } else if (axis == 2) {
      out_color.rgb = gizmo.color_z;
   }

   int highlight_flag = (1 << 4) << axis;
   if ((gizmo.flags & highlight_flag) == highlight_flag) {
      out_color.rgb = mix(out_color.rgb, gizmo.color_highlight.rgb, gizmo.color_highlight.a);
   }
}