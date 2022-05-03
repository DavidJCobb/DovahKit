#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : enable

#include "../../includes/structs/scene_global_state.glsl"
#include "../../includes/structs/rendered_bounds_shader_params.glsl"

layout(std430,set=0,binding=0) uniform UniformBufferObject {
   scene_global_state scene;
};
layout(std430,set=1,binding=0) readonly buffer ObjectBuffer {
	rendered_bounds_shader_params scene_bounds[];
};

layout(location = 0) out VS_OUT {
   vec4 out_color;
};

#define AXIS_COUNT     3
#define LINES_PER_AXIS 4
#define VERTS_PER_LINE 2
#define VERTS_PER_AXIS (LINES_PER_AXIS * VERTS_PER_LINE)
// auto& line_vert = all_verts[(current_axis * lines_per_axis) + (current_line * verts_per_line) + current_vert];

void main() {
   mat4 transform = scene_bounds[gl_InstanceIndex].transform;
   //
   int axis_index = gl_VertexIndex / VERTS_PER_AXIS;
   int vert_index = gl_VertexIndex % VERTS_PER_LINE;
   int line_index = (gl_VertexIndex % VERTS_PER_AXIS) / VERTS_PER_LINE;
   //
   int axis_a = (axis_index + 1) % AXIS_COUNT;
   int axis_b = (axis_index + 2) % AXIS_COUNT;
   gl_Position[axis_index] = ((vert_index)     * 2) - 1; // -1 or 1
   gl_Position[axis_a]     = ((line_index / 2) * 2) - 1; // [0, 1, 2, 3] -> [0, 0, 2, 2] -> [-1, -1,  1,  1]
   gl_Position[axis_b]     = ((line_index % 2) * 2) - 1; // [0, 1, 2, 3] -> [0, 2, 0, 2] -> [-1,  1, -1,  1]
   gl_Position.w = 1;
   //
   gl_Position = scene.proj * scene.view * transform * gl_Position;
   //
   out_color = vec4(0, 0, 0, 1);
   out_color[axis_index] = min(1, vert_index + 0.5);
}