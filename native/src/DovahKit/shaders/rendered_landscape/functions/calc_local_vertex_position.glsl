
#ifndef INCLUDE_GUARD_calc_local_vertex_position // include guard
#define INCLUDE_GUARD_calc_local_vertex_position

#define LANDSCAPE_QUAD_SIDE_VERTS  17
#define LANDSCAPE_QUAD_TOTAL_VERTS (LANDSCAPE_QUAD_SIDE_VERTS * LANDSCAPE_QUAD_SIDE_VERTS)
#define LANDSCAPE_QUAD_SIDE_OFFSET 16

#define LANDSCAPE_CELL_SIDE_LENGTH 4096
#define LANDSCAPE_QUAD_SIDE_LENGTH (LANDSCAPE_CELL_SIDE_LENGTH / 2)
#define LANDSCAPE_VERT_DISTANCE (LANDSCAPE_QUAD_SIDE_LENGTH / (LANDSCAPE_QUAD_SIDE_VERTS - 1))

vec3 calc_local_vertex_position(int quad, int vertex_index, float height) {
   vertex_index = vertex_index % LANDSCAPE_QUAD_TOTAL_VERTS; // allows for sharing a vertex buffer and shifting the base vertex offset
   int land_x = vertex_index % LANDSCAPE_QUAD_SIDE_VERTS + (LANDSCAPE_QUAD_SIDE_OFFSET * (quad % 2));
   int land_y = vertex_index / LANDSCAPE_QUAD_SIDE_VERTS + (LANDSCAPE_QUAD_SIDE_OFFSET * (quad / 2));
   //
   return vec3(
      land_x * LANDSCAPE_VERT_DISTANCE,
      land_y * LANDSCAPE_VERT_DISTANCE,
      height
   );
}

#endif