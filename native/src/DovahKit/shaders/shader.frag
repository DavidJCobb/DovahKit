#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(push_constant) uniform PER_OBJECT {
   int object_index;
	int texture_index;
} pushed;

layout(std140,binding = 0) uniform UniformBufferObject {
   mat4 view;
   mat4 proj;
   vec3 ambient_light_color;
   vec3 sun_pos;
   vec3 sun_color;
} ubo;
layout(binding = 1) uniform sampler   texSampler;
// binding 2 is used by the vertex shader (buffer for object data)
layout(binding = 3) uniform texture2D textures[];

layout(location = 0) in VS_OUT {
   vec3 color;
   vec2 uv;
   vec3 normal;
   vec3 pos_world;
} fs_in;

layout(location = 0) out vec4 outColor;

void main() {
   outColor  = texture(sampler2D(textures[pushed.texture_index], texSampler), fs_in.uv);
   outColor *= vec4(fs_in.color, 1.0);
   //
   vec3  norm    = normalize(fs_in.normal);
   vec3  sun_dir = normalize(ubo.sun_pos - fs_in.pos_world);
   float diff    = max(dot(norm, sun_dir), 0.0);
   vec3  diffuse = diff * ubo.sun_color;
   //
   outColor = vec4(ubo.ambient_light_color + diffuse, 1.0) * outColor;
}