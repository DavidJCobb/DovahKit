
#include "../structs/rendered_mesh_shader_params.glsl"

#define DECLARE_ALL_MESHES_PARAMS \
	layout(std140,set=SET_ALL_MESHES,binding=0) readonly buffer AllMeshes { rendered_mesh_shader_params scene_meshes[]; };