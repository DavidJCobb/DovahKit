#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : enable

#include "../includes/structs/scene_global_state.glsl"
#include "../includes/structs/rendered_landscape_shader_params.glsl"

#include "../includes/descriptor_sets/all_landscapes.glsl"
#include "../includes/descriptor_sets/scene_state.glsl"

#define SET_SCENE_STATE    0
#define SET_ALL_LANDSCAPES 1

DECLARE_SCENE_STATE_PARAMS    // scene
DECLARE_ALL_LANDSCAPES_PARAMS // landscapes[]

layout(location = 0) out vec4 out_color;

void main() {
   out_color = vec4(0, 1, 0, 1);
}