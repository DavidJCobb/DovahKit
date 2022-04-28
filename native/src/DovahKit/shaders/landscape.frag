#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : enable

// binding 0: scene_global_state
// binding 1: rendered landscape shader parameters array

layout(location = 0) in VS_OUT {
   vec4 in_color;
   vec3 in_world_pos;
};

layout(location = 0) out vec4 out_color;

void main() {
   out_color = in_color;
}