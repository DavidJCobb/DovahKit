#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : enable

#define USE_ALPHA 0
#include "cores/standard_shader.frag"

layout(location = 0) out vec4 out_color;

void main() {
   out_color = calculate_color();
}