#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : enable

#include "../../includes/structs/scene_global_state.glsl"

#include "../../includes/descriptor_sets/scene_state.glsl"

#define SET_SCENE_STATE    0

DECLARE_SCENE_STATE_PARAMS // scene

#include "color.fragment-input.glsl"
layout(location = 0) out VS_OUT {
   fragment_input vs_out;
};

#define GRID_EXTENT 4096

void main() {
   gl_Position = vec4(GRID_EXTENT, GRID_EXTENT, 0, 1);
   if (gl_VertexIndex < 2)
      gl_Position.y *= -1.0;
   if (gl_VertexIndex % 3 == 0)
      gl_Position.x *= -1.0;
   
   vs_out.pos_world = gl_Position.xyz;

   gl_Position = scene.proj * scene.view * gl_Position;
}