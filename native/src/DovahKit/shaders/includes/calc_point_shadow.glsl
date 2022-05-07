
#ifndef INCLUDE_GUARD_calc_point_shadow
#define INCLUDE_GUARD_calc_point_shadow

#define USE_POINT_SHADOW_DEPTH_BIAS 1
#define USE_INVERTED_POINT_SHADOW_MAP 0

// shadow values range from 0 (bright) to 1 (shadowed)
float calc_point_shadow(
   vec3        normal,               // surface normal
   vec3        light_dir,            // light direction, in surface tangent space
   float       light_distance_ratio, // distance to light / light radius
   vec3        vector_to_light,      // non-normalized vector to the light
   samplerCube shadow_map
) {
   #if USE_POINT_SHADOW_DEPTH_BIAS
      float bias = max(0.05 * (1.0 - dot(normal, light_dir)), 0.005); // a small offset is needed to prevent self-shadowing
   #else
      const float bias = 0.0;
   #endif

   // NOTE: A side effect of applying a depth bias is that the "bias" value effectively 
   //       becomes our minimum possible depth granularity. If bias is 0.005, for example, 
   //       then two depth values within 0.0049 of each other are impossible to tell apart.
   
   float closest_depth = texture(shadow_map, vector_to_light).r;
   float shadow;
   #if USE_INVERTED_POINT_SHADOW_MAP == 1
      shadow = light_distance_ratio + bias < closest_depth ? 1.0 : 0.0;
   #else
      shadow = light_distance_ratio - bias > closest_depth ? 1.0 : 0.0;
   #endif
   
   return shadow;
}

#endif