#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : enable

#include "../includes/structs/scene_global_state.glsl"

#include "../includes/descriptor_sets/scene_state.glsl"

#define SET_SCENE_STATE 0

DECLARE_SCENE_STATE_PARAMS // scene

layout (points) in;
layout (line_strip, max_vertices = 2) out;

layout(location = 0) in VS_OUT {
   vec3 normal;
} gs_in[]; 

void main() {    
    gl_Position = scene.proj * scene.view * (gl_in[0].gl_Position);
    EmitVertex();

    gl_Position = scene.proj * scene.view * (gl_in[0].gl_Position + vec4(gs_in[0].normal * 96, 0));
    EmitVertex();
    
    EndPrimitive();
}  