
#ifndef INCLUDE_GUARD_rendered_landscape_shader_params
#define INCLUDE_GUARD_rendered_landscape_shader_params

struct rendered_landscape_shader_params {
   vec3 position; // world-relative position of the southwest corner vertex. landscapes are never rotated or scaled, so a full matrix is not necessary
   int  pad0C;
   int  diffuse_base[4]; // one per quad
   int  normals_base[4];
   int  diffuse_blends[6 * 4]; // six per quad
   int  normals_blends[6 * 4];
};

#endif