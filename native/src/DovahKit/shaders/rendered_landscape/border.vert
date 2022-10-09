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

layout(location = 0) out VS_OUT {
   flat int vs_out_is_alt; // TODO: don't make flat; interp within the frag shader
};

void main() {
   int vert_i33 = gl_VertexIndex % (34 * 34);
   int vert_i   = gl_VertexIndex % (17 * 17);

   int quad   = vert_i33 / LANDSCAPE_QUAD_TOTAL_VERTS;
   int land_x = vert_i % LANDSCAPE_QUAD_SIDE_VERTS + (LANDSCAPE_QUAD_SIDE_OFFSET * (quad % 2));
   int land_y = vert_i / LANDSCAPE_QUAD_SIDE_VERTS + (LANDSCAPE_QUAD_SIDE_OFFSET * (quad / 2));

   int landscape_index = gl_InstanceIndex;

   vs_out_is_alt = 0;
   if ((land_x % 2) != (land_y % 2)) {
      vs_out_is_alt = 1;
   }

   vec3 pos_world = vec3(
      land_x * LANDSCAPE_VERT_DISTANCE,
      land_y * LANDSCAPE_VERT_DISTANCE,
      in_height
   );
   pos_world += landscapes[landscape_index].position;
   gl_Position = scene.proj * scene.view * vec4(pos_world, 1);
}