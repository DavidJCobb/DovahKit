
#ifndef INCLUDE_GUARD_calc_point_shadow
#define INCLUDE_GUARD_calc_point_shadow

#define USE_POINT_SHADOW_DEPTH_BIAS 1
#define USE_POINT_SHADOW_PCF 1
#define USE_INVERTED_POINT_SHADOW_MAP 0

// shadow values range from 0 (bright) to 1 (shadowed)
float calc_point_shadow(
   vec3        normal,               // surface normal
   vec3        normal_geo_only,      // surface normal NOT including normal map
   vec3        light_dir,            // light direction, in surface tangent space
   float       light_distance_ratio, // distance to light / light radius
   vec3        vector_to_light,      // non-normalized vector to the light
   samplerCube shadow_map
) {
   #if USE_POINT_SHADOW_DEPTH_BIAS
      //
      // A small offset is needed to prevent self-shadowing and "shadow acne." The formula 
      // here scales the depth bias based on the angle between the light and the surface: 
      // the more directly the light strikes the surface, the smaller the bias.
      //
      // Because we want to scale the bias based on the physical surface, we need the 
      // normal vector from the geometry, without influence from the normal map texture.
      //
      float bias = max(0.05 * (1.0 - dot(normal_geo_only, light_dir)), 0.005);
   #else
      const float bias = 0.0;
   #endif

   // NOTE: A side effect of applying a depth bias is that the "bias" value effectively 
   //       becomes our minimum possible depth granularity. If bias is 0.005, for example, 
   //       then two depth values within 0.0049 of each other are impossible to tell apart.
   
   #if USE_POINT_SHADOW_PCF
      /*//
      ivec2 tex_size = textureSize(shadow_map, 0);
	   float scale    = 1.0;
	   float dx       = scale / float(tex_size.x);
	   float dy       = scale / float(tex_size.y);
      //*/

	   const int range = 1; // [-range, range] on each axis
      const int count = (2 * range + 1) * (2 * range + 1);

      vec3 axis_v = normalize(vector_to_light);
      vec3 axis_a = vec3(1, 0, 0);
      vec3 axis_b = cross(axis_v, axis_a);
      axis_a = cross(axis_v, axis_b);
	
	   float shadow = 0.0;
	   for (int x = -range; x <= range; x++)  {
		   for (int y = -range; y <= range; y++) {
			   float pcf_depth = texture(shadow_map, vector_to_light + (x * axis_a + y * axis_b)).r;
            #if USE_INVERTED_POINT_SHADOW_MAP == 1
               shadow += light_distance_ratio + bias < pcf_depth ? 1.0 : 0.0;
            #else
               shadow += light_distance_ratio - bias > pcf_depth ? 1.0 : 0.0;
            #endif
		   }
	   }
	   shadow /= count;
   #else
      float closest_depth = texture(shadow_map, vector_to_light).r;
      float shadow;
      #if USE_INVERTED_POINT_SHADOW_MAP == 1
         shadow = light_distance_ratio + bias < closest_depth ? 1.0 : 0.0;
      #else
         shadow = light_distance_ratio - bias > closest_depth ? 1.0 : 0.0;
      #endif
   #endif
   
   return shadow;
}

#endif