#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : enable

#include "../includes/structs/scene_global_state.glsl"
#include "../includes/structs/rendered_landscape_shader_params.glsl"

#include "../includes/descriptor_sets/all_landscapes.glsl"
#include "../includes/descriptor_sets/scene_state.glsl"

#include "vertex-inputs.glsl"

#define SET_SCENE_STATE    0
#define SET_ALL_LANDSCAPES 1

DECLARE_SCENE_STATE_PARAMS    // scene
DECLARE_ALL_LANDSCAPES_PARAMS // landscapes[]

#include "functions/calc_local_vertex_position.glsl"

void main() {
   int quad = gl_InstanceIndex % 4;
   int landscape_index = gl_InstanceIndex / 4;
   
   vec3 pos_world = calc_local_vertex_position(quad, gl_VertexIndex, in_height) + landscapes[landscape_index].position;
   gl_Position = scene.sun_space * vec4(pos_world, 1);

}