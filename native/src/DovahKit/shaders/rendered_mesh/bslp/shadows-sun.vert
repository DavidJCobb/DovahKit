#version 450
#extension GL_GOOGLE_include_directive : enable

#include "../../includes/structs/rendered_mesh_push_constant.glsl"
#include "../../includes/structs/rendered_mesh_shader_params.glsl"
#include "../../includes/structs/scene_global_state.glsl"

#include "../../includes/standard_vertex_inputs.glsl"

#include "shadows-sun.descriptors.glsl"
DECLARE_SCENE_STATE_PARAMS
DECLARE_ALL_MESHES_PARAMS

layout(location = 0) out VS_OUT {
   vec2 uv;
} vs_out;

void main() {
   mat4 model_transform = scene_meshes[pushed.object_index].transform;
   //
	gl_Position = (scene.sun_space * model_transform) * vec4(in_position, 1.0);
   vs_out.uv   = in_uv;
}