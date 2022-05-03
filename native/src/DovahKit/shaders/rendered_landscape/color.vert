#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : enable

#include "../includes/structs/scene_global_state.glsl"
#include "../includes/structs/rendered_landscape_shader_params.glsl"

#include "../includes/descriptor_sets/all_landscapes.glsl"
#include "../includes/descriptor_sets/all_textures.glsl"
#include "../includes/descriptor_sets/scene_state.glsl"

layout(location = 0) in vec3  in_color;
layout(location = 1) in float in_height;
layout(location = 2) in float blends[6];

#define SET_SCENE_STATE    0
#define SET_ALL_TEXTURES   1
#define SET_ALL_LANDSCAPES 2
#define SET_ALL_LIGHTS     3

DECLARE_SCENE_STATE_PARAMS    // scene
DECLARE_ALL_LANDSCAPES_PARAMS // landscapes[]

layout(location = 0) out VS_OUT {
   vec4     out_color;
   vec3     out_world_pos;
   flat int out_quad;
   float    out_blends[6];
   vec2     out_uv;
   flat int out_landscape_index;
};

#define VERTS_PER_SIDE   17
#define TOTAL_VERTS      (VERTS_PER_SIDE * VERTS_PER_SIDE)
#define QUAD_SIDE_OFFSET 16

#define CELL_SIDE_LENGTH 4096
#define QUAD_SIDE_LENGTH (CELL_SIDE_LENGTH / 2)
#define VERT_DISTANCE (QUAD_SIDE_LENGTH / (VERTS_PER_SIDE - 1))

void main() {
   out_quad = gl_InstanceIndex % 4;
   out_landscape_index = gl_InstanceIndex / 4;

   int vert_i = gl_VertexIndex % TOTAL_VERTS; // allows for sharing a vertex buffer and shifting the base vertex offset
   int land_x = vert_i % VERTS_PER_SIDE + (QUAD_SIDE_OFFSET * (out_quad % 2));
   int land_y = vert_i / VERTS_PER_SIDE + (QUAD_SIDE_OFFSET * (out_quad / 2));
   //
   gl_Position.x = land_x * VERT_DISTANCE;
   gl_Position.y = land_y * VERT_DISTANCE;
   gl_Position.z = in_height;
   gl_Position.w = 1;
   //
   out_color = vec4(in_color, 1);
   //
   gl_Position += vec4(landscapes[out_landscape_index].position, 0);
   out_world_pos = vec3(gl_Position);
   gl_Position = scene.proj * scene.view * gl_Position;
   //
   for(int i = 0; i < 6; ++i)
      out_blends[i] = blends[i];
   out_uv = vec2(land_x, land_y);
}