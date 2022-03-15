
// #include this from a main shader, which defines outputs
//
// your shader's main() can return the result of calculate_color() verbatim
// or
// your shader can use it in other calculations (e.g. OIT)

#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : enable

#include "../includes/calc_specular_strength.glsl"
#include "../includes/calc_directional_shadow.glsl"
#include "../includes/computed_light.glsl"
#include "../includes/calc_directional_light.glsl"
#include "../includes/point_light.glsl"
#include "../includes/calc_point_light.glsl"

layout (constant_id = 0) const int MAX_LIGHTS = 4;

layout(push_constant) uniform PER_OBJECT {
   int object_index;
	int texture_index;
   int texture_normal_index;
} pushed;

struct ObjectData {
	mat4  transform;
   vec3  specular_color;
   float specular_strength;
   float specular_exponent;
};

layout(std140,binding = 0) uniform UniformBufferObject {
   mat4 view;
   mat4 proj;
   vec3 ambient_light_color;
   vec3 sun_dir;
   vec3 sun_color;
	mat4 sun_space;
} ubo;
layout(binding = 1) uniform sampler texSampler;
layout(binding = 2) uniform sampler2D shadowMap;
layout(std140,set = 0, binding = 3) readonly buffer ObjectBuffer {
	ObjectData objects[];
} objectBuffer;
layout(std140,set = 0, binding = 4) readonly buffer PointLightBuffer {
	point_light lights[MAX_LIGHTS];
} pointLightBuffer;
layout(binding = 5) uniform texture2D textures[];

layout(location = 0) in VS_OUT {
   vec3  color;
   vec2  uv;
   vec3  pos_world;
   mat3  tangent_space;
   vec3  tangent_sun_dir;
   vec3  tangent_view_pos;
   vec3  tangent_vert_pos;
   vec4  sun_shadow_vert_pos;
   float camera_distance;
} fs_in;

vec4 calculate_color() {
   vec4 color;
   //
   ObjectData current_object = objectBuffer.objects[pushed.object_index];
   //
   color  = texture(sampler2D(textures[pushed.texture_index], texSampler), fs_in.uv);
   color *= vec4(fs_in.color, 1.0);
   //
   vec3 normal = vec3(0, 0, 1);
   if (pushed.texture_normal_index >= 0) {
      normal = texture(sampler2D(textures[pushed.texture_normal_index], texSampler), fs_in.uv).rgb;
      normal = normalize(normal * 2.0 - 1.0);
   }
   //
   // Apply all lights in the scene:
   //
   vec3 view_dir = normalize(fs_in.tangent_view_pos - fs_in.tangent_vert_pos); // direction from camera position to fragment position
   //
   computed_light light_data = calc_directional_light(
      fs_in.tangent_sun_dir,
      fs_in.sun_shadow_vert_pos,
      ubo.sun_color,
      normal,
      view_dir,
      current_object.specular_exponent,
      shadowMap
   );
   for(int i = 0; i < MAX_LIGHTS; ++i) {
      computed_light current = calc_point_light(
         pointLightBuffer.lights[i],
         fs_in.tangent_space,
         normal,
         fs_in.tangent_vert_pos,
         view_dir,
         current_object.specular_exponent
      );
      light_data.diffuse  += current.diffuse;
      light_data.specular += current.specular;
   }
   light_data.specular *= current_object.specular_strength * current_object.specular_color;
   //
   color = vec4(ubo.ambient_light_color + light_data.diffuse + light_data.specular, 1.0) * color;
   return color;
}