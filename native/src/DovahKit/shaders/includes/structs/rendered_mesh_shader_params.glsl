
#ifndef INCLUDE_GUARD_rendered_mesh_shader_params
#define INCLUDE_GUARD_rendered_mesh_shader_params

struct rendered_mesh_shader_params {
	mat4  transform;
   //
   vec3  specular_color;
   float specular_strength;
   float specular_exponent;
   //
   vec3  bounding_sphere_center;
   float bounding_sphere_radius;
};

#endif