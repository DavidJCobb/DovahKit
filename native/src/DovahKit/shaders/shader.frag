#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(push_constant) uniform PER_OBJECT {
   int object_index;
	int texture_index;
} pushed;

struct ObjectData{
	mat4  transform;
   vec3  specular_color;
   float specular_strength;
   float specular_exponent;
};

layout(std140,binding = 0) uniform UniformBufferObject {
   mat4 view;
   mat4 proj;
   vec3 ambient_light_color;
   vec3 sun_pos;
   vec3 sun_color;
} ubo;
layout(binding = 1) uniform sampler   texSampler;
layout(std140,set = 0, binding = 2) readonly buffer ObjectBuffer {
	ObjectData objects[];
} objectBuffer;
layout(binding = 3) uniform texture2D textures[];

layout(location = 0) in VS_OUT {
   vec3 color;
   vec2 uv;
   vec3 normal;
   vec3 pos_world;
} fs_in;

layout(location = 0) out vec4 outColor;

void main() {
   ObjectData current_object = objectBuffer.objects[pushed.object_index];
   //
   outColor  = texture(sampler2D(textures[pushed.texture_index], texSampler), fs_in.uv);
   outColor *= vec4(fs_in.color, 1.0);
   //
   vec3  norm    = normalize(fs_in.normal);
   vec3  sun_dir = normalize(ubo.sun_pos - fs_in.pos_world);
   float diff    = max(dot(norm, sun_dir), 0.0);
   vec3  diffuse = diff * ubo.sun_color;
   //
   // Specular (needs to be done per light source, I guess):
   //
   vec3  view_dir    = normalize(vec3(ubo.view[3]) - fs_in.pos_world); // direction from camera position to fragment position
   vec3  reflect_dir = reflect(-sun_dir, norm);
   float spec_str    = pow(max(dot(view_dir, reflect_dir), 0.0), current_object.specular_exponent);
   vec3  specular    = current_object.specular_strength * spec_str * current_object.specular_color * ubo.sun_color;
   //
   outColor = vec4(ubo.ambient_light_color + diffuse + specular, 1.0) * outColor;
}