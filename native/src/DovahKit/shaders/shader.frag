#version 450
#extension GL_EXT_nonuniform_qualifier : require

#define BLINN_PHONG_MODE_PHONG 0
#define BLINN_PHONG_MODE_BLINN 1
//
#define BLINN_PHONG_MODE BLINN_PHONG_MODE_BLINN

layout (constant_id = 0) const int MAX_LIGHTS = 4;

layout(push_constant) uniform PER_OBJECT {
   int object_index;
	int texture_index;
   int texture_normal_index;
} pushed;

struct PointLightData {
   mat4  transform;
   vec3  color;
   float radius;
   float fade;
};
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
   vec3 sun_dir;
   vec3 sun_color;
} ubo;
layout(binding = 1) uniform sampler   texSampler;
layout(std140,set = 0, binding = 2) readonly buffer ObjectBuffer {
	ObjectData objects[];
} objectBuffer;
layout(std140,set = 0, binding = 3) readonly buffer PointLightBuffer {
	PointLightData lights[MAX_LIGHTS];
} pointLightBuffer;
layout(binding = 4) uniform texture2D textures[];

layout(location = 0) in VS_OUT {
   vec3 color;
   vec2 uv;
   vec3 pos_world;
   mat3 tangent_space;
   vec3 tangent_sun_dir;
   vec3 tangent_view_pos;
   vec3 tangent_vert_pos;
} fs_in;

layout(location = 0) out vec4 outColor;

// all arguments are in tangent space
float calc_specular_strength(vec3 normal, vec3 light_dir, vec3 view_dir, float specular_exponent) {
   #if BLINN_PHONG_MODE == BLINN_PHONG_MODE_PHONG
      vec3 reflect_dir = reflect(-light_dir, normal);
      return pow(max(dot(view_dir, reflect_dir), 0.0), specular_exponent);
   #else
      #if BLINN_PHONG_MODE == BLINN_PHONG_MODE_BLINN
         vec3  halfway_dir = normalize(light_dir + view_dir);
         return pow(max(dot(normal, halfway_dir), 0.0), specular_exponent);
      #else
         #error Unrecognized BLINN_PHONG_MODE.
      #endif
   #endif
}

struct computed_light {
   vec3 diffuse;  // light color and diffuse  strength; multiply the object's diffuse color into this
   vec3 specular; // light color and specular strength; multiply the object's specular strength and color into this
};

// inputs except (light) are in tangent space, where applicable
computed_light calc_point_light(PointLightData light, vec3 normal, vec3 vert_pos, vec3 view_dir, float specular_exponent) {
   computed_light result;
   //
   vec3  light_pos = fs_in.tangent_space * vec3(light.transform[3]);
   vec3  light_dir = normalize(light_pos - vert_pos);
   float str_diff  = max(dot(light_dir, normal), 0.0); // diffuse strength
   float str_spec  = calc_specular_strength(normal, light_dir, view_dir, specular_exponent);
   //
   float distance  = length(light_pos - vert_pos);
   float attenuate = light.fade / (distance * distance);
   //
   str_diff *= attenuate;
   str_spec *= attenuate;
   //
   result.diffuse  = str_diff * light.color;
   result.specular = str_spec * light.color;
   return result;
}

// inputs are in tangent space, where applicable
computed_light calc_directional_light(vec3 light_dir, vec3 light_color, vec3 normal, vec3 view_dir, float specular_exponent) {
   computed_light result;
   //
   light_dir = normalize(-light_dir);
   //
   float str_diff = max(dot(normal, light_dir), 0.0);
   float str_spec = calc_specular_strength(normal, light_dir, view_dir, specular_exponent);
   //
   result.diffuse  = str_diff * light_color;
   result.specular = str_spec * light_color;
   return result;
}

void main() {
   ObjectData current_object = objectBuffer.objects[pushed.object_index];
   //
   outColor  = texture(sampler2D(textures[pushed.texture_index], texSampler), fs_in.uv);
   outColor *= vec4(fs_in.color, 1.0);
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
      ubo.sun_color,
      normal,
      view_dir,
      current_object.specular_exponent
   );
   for(int i = 0; i < MAX_LIGHTS; ++i) {
      computed_light current = calc_point_light(pointLightBuffer.lights[i], normal, fs_in.tangent_vert_pos, view_dir, current_object.specular_exponent);
      light_data.diffuse  += current.diffuse;
      light_data.specular += current.specular;
   }
   light_data.specular *= current_object.specular_strength * current_object.specular_color;
   //
   outColor = vec4(ubo.ambient_light_color + light_data.diffuse + light_data.specular, 1.0) * outColor;

   /*
   //
   vec3  sun_dir = normalize(fs_in.tangent_sun_pos - fs_in.tangent_vert_pos);
   float diff    = max(dot(sun_dir, normal), 0.0);
   vec3  diffuse = diff * ubo.sun_color;
   //
   // Specular (needs to be done per light source, I guess):
   //
   vec3  view_dir = normalize(fs_in.tangent_view_pos - fs_in.tangent_vert_pos); // direction from camera position to fragment position
   float str_spec = calc_specular_strength(normal, sun_dir, view_dir, current_object.specular_exponent);
   vec3  specular = current_object.specular_strength * str_spec * current_object.specular_color;
   //
   outColor = vec4(ubo.ambient_light_color + diffuse + specular, 1.0) * outColor;
   //*/
}