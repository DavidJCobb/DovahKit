#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : enable

// binding 0: scene_global_state
// binding 1: bounding box matrix list

layout(location = 0) out vec4 out_color;

void main() {
   out_color = vec4(1, 1, 0, 1);
}