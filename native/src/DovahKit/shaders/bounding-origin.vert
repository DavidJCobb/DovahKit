#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : enable

#include "includes/scene_global_state.glsl"

layout(std430,binding = 0) uniform UniformBufferObject {
   scene_global_state scene;
};
layout(std430,binding = 1) readonly buffer ObjectBuffer {
	mat4 scene_bounding_boxes[];
};

#define MARKER_SIZE 16

#define AXIS_COUNT     3
#define LINES_PER_AXIS 1
#define VERTS_PER_LINE 2
#define VERTS_PER_AXIS (LINES_PER_AXIS * VERTS_PER_LINE)
// auto& line_vert = all_verts[(current_axis * lines_per_axis) + (current_line * verts_per_line) + current_vert];

void main() {
   mat4 transform = scene_bounding_boxes[gl_InstanceIndex];
   transform[0] = normalize(transform[0]); // remove scaling
   transform[1] = normalize(transform[1]);
   transform[2] = normalize(transform[2]);
   //
   int axis_index = gl_VertexIndex / VERTS_PER_LINE;
   int vert_index = gl_VertexIndex % VERTS_PER_LINE;
   //
   gl_Position = vec4(0, 0, 0, 1);
   gl_Position[axis_index] = ((vert_index) * (MARKER_SIZE * 2)) - MARKER_SIZE; // -MARKER_SIZE or +MARKER_SIZE
   gl_Position.w = 1;
   //
   gl_Position = scene.proj * scene.view * transform * gl_Position;
}