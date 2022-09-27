#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : enable

#include "../includes/structs/scene_global_state.glsl"

#include "../includes/descriptor_sets/scene_state.glsl"

#define SET_SCENE_STATE    0

DECLARE_SCENE_STATE_PARAMS    // scene

layout(location = 0) out vec4 out_color;

void main() {
   out_color = vec4(scene.fog_color_far, 1.0);
}