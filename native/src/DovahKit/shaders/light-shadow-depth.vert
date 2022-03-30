#version 450
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_multiview : enable

#include "includes/point_light.glsl"
#include "includes/rendered_mesh_push_constant.glsl"
#include "includes/rendered_mesh_shader_params.glsl"
#include "includes/scene_global_state.glsl"

#include "includes/standard_vertex_inputs.glsl"

layout (constant_id = 0) const int CASTER_INDEX = 0;
layout (constant_id = 2) const int MAX_LIGHTS   = 4;

layout(std140,binding = 0) uniform UniformBufferObject {
   scene_global_state ubo;
};
layout(std430,set = 0, binding = 1) readonly buffer ObjectBuffer {
	rendered_mesh_shader_params objects[];
} objectBuffer;
layout(std430,set = 0, binding = 2) readonly buffer PointLightBuffer {
	point_light lights[MAX_LIGHTS];
} pointLightBuffer;
layout(std430,set = 0, binding = 3) readonly buffer LightViewProjMatrices {
   mat4 light_space_matrices[4][6];
};
// binding 4: texture sampler
// binding 5: texture array

layout(location = 0) out VS_OUT {
   vec2  uv;
   float distance;
} vs_out;

out gl_PerVertex {
   vec4 gl_Position;
};

void main() {
   mat4 model_transform = objectBuffer.objects[pushed.object_index].transform;
   //
   vec4 light_pos;
   {
      int light_index = ubo.shadow_caster_index[CASTER_INDEX];
      if (light_index >= 0) {
         light_pos = pointLightBuffer.lights[light_index].transform[3];
      }
   }
   mat4 light_space = light_space_matrices[CASTER_INDEX][gl_ViewIndex];
   //
	gl_Position = (light_space * model_transform) * vec4(in_position, 1.0);
   vs_out.uv   = in_uv;
   vs_out.distance = length((model_transform) * vec4(in_position, 1.0) - light_pos);
}