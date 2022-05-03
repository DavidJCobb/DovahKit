struct rendered_landscape_shader_params {
   vec3 position;
   int  pad0C;
   int  diffuse_base[4];
   int  normals_base[4];
   int  diffuse_blends[6 * 4];
   int  normals_blends[6 * 4];
};