#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : enable

layout(location = 0) in VS_OUT {
   flat int vs_out_is_alt;
};
layout(location = 0) out vec4 out_color;

#include "functions/calc_local_vertex_position.glsl"

void main() {
   out_color = vec4(1, 1, 0, 1);
   if (vs_out_is_alt != 0) { // TODO: interp color instead
      out_color = vec4(0, 0, 0, 1);
   }
}