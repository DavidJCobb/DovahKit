#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : enable

#include "includes/scene_global_state.glsl"
#include "includes/rendered_landscape_shader_params.glsl"

layout(location = 0) in vec3  in_color;
layout(location = 1) in float in_height;
layout(location = 2) in float blends[6];

layout(std430,binding = 0) uniform UniformBufferObject {
   scene_global_state scene;
};
layout(std430,binding = 1) readonly buffer ObjectBuffer {
	rendered_landscape_shader_params landscapes[];
};

layout(location = 0) out VS_OUT {
   vec4 out_color;
   vec3 out_world_pos;
};

#define VERTS_PER_SIDE 33
#define TOTAL_VERTS    (VERTS_PER_SIDE * VERTS_PER_SIDE)
#define VERTS_PER_QUAD 17

#define CELL_SIDE_LENGTH 4096
#define VERT_DISTANCE (CELL_SIDE_LENGTH / (VERTS_PER_SIDE - 1))

void main() {
   mat4 transform = mat4(1);
   transform[3] = vec4(landscapes[gl_InstanceIndex].position, 1.0);
   //
   int vert_i = gl_VertexIndex % TOTAL_VERTS; // allows for sharing a vertex buffer and shifting the base vertex offset
   int land_x = vert_i % VERTS_PER_SIDE;
   int land_y = vert_i / VERTS_PER_SIDE;
   //
   gl_Position.x = land_x * VERT_DISTANCE;
   gl_Position.y = land_y * VERT_DISTANCE;
   gl_Position.z = in_height;
   gl_Position.w = 1;
   //
   out_color = vec4(0.5, 0.5, 0.5, 1); // testing
   //
   out_world_pos = (transform * gl_Position).xyz;
   gl_Position = scene.proj * scene.view * transform * gl_Position;
}