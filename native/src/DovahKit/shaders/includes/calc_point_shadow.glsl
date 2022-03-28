
#define USE_POINT_SHADOW_DEPTH_BIAS 0
#define USE_SHADOW_PCF 1
#define USE_INVERTED_SHADOW_MAP 0

float calc_point_shadow(
   vec3        normal,          // surface normal
   vec3        light_dir,       // light direction, in surface tangent space
   vec3        vector_to_light, // non-normalized vector to the light
   samplerCube shadow_map
) {
   float distance_to_light = length(vector_to_light);
   #if USE_POINT_SHADOW_DEPTH_BIAS
      float bias = max(0.05 * (1.0 - dot(normal, light_dir)), 0.005); // a small offset is needed to prevent self-shadowing
   #else
      const float bias = 0.0;
   #endif

   // NOTE: A side effect of applying a depth bias is that the "bias" value effectively 
   //       becomes our minimum possible depth granularity. If bias is 0.005, for example, 
   //       then two depth values within 0.0049 of each other are impossible to tell apart.

   #if USE_SHADOW_PCF
      ivec2 tex_size = textureSize(shadow_map, 0);
	   float scale    = 1.5;
	   float dx       = scale / float(tex_size.x);
	   float dy       = scale / float(tex_size.y);

	   const int range = 1; // [-range, range] on each axis
      const int count = (2 * range + 1) * (2 * range + 1);
	
	   float shadow = 0.0;
	   for (int x = -range; x <= range; x++)  {
		   for (int y = -range; y <= range; y++) {
			   float pcf_depth = texture(shadow_map, vector_to_light).r;
            #if USE_INVERTED_SHADOW_MAP == 1
               shadow += distance_to_light + bias < pcf_depth ? 1.0 : 0.0;
            #else
               shadow += distance_to_light - bias > pcf_depth ? 1.0 : 0.0;
            #endif
		   }
	   }
	   shadow /= count;
   #else
      float closest_depth = texture(shadow_map, vector_to_light).r; // distance from the light to the nearest surface along this angle
      float shadow;
      #if USE_INVERTED_SHADOW_MAP == 1
         shadow = distance_to_light + bias < closest_depth ? 1.0 : 0.0;
      #else
         shadow = distance_to_light - bias > closest_depth ? 1.0 : 0.0;
      #endif
   #endif
   //
   return shadow;
}