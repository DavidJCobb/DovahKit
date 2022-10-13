#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : enable

#include "color-base.frag.glsl"

layout(location = 0) out vec4 out_color;

void main() {
   out_color = calculate_color();
   //
   // Do not use alpha transparency to anti-alias if invoked as non-OIT:
   //
   if (out_color.a < 0.5) {
      discard;
   }
   out_color.a = 1.0;
}