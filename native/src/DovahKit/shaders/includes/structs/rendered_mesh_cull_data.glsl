struct rendered_mesh_cull_data {
	mat4  transform;
   vec3  bounding_sphere_center;
   float bounding_sphere_radius;
   int   flags;
};

#define RENDERED_MESH_CULL_FLAG_CULLED_BY_APPLICATION 0x00000001