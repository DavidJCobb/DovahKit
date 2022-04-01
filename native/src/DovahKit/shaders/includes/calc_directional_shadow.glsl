
#ifndef included_calc_directional_shadow // include guard
#define included_calc_directional_shadow

#define USE_SHADOW_DEPTH_BIAS 1
#define USE_SHADOW_PCF 1
#define USE_INVERTED_SHADOW_MAP 0

// shadow values range from 0 (bright) to 1 (shadowed)
float calc_directional_shadow(
   vec3      normal,          // surface normal
   vec3      light_dir,       // light direction, in surface tangent space
   vec4      light_space_pos, // light-space vertex position
   sampler2D shadow_map
) {
   vec3  proj_coord    = light_space_pos.xyz / light_space_pos.w; // perspective divide
   float current_depth = proj_coord.z; // distance from the light to the current vertex
   #if USE_SHADOW_DEPTH_BIAS
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
			   float pcf_depth = texture(shadow_map, proj_coord.xy + vec2(x * dx, y * dy)).r;
            #if USE_INVERTED_SHADOW_MAP == 1
               shadow += current_depth + bias < pcf_depth ? 1.0 : 0.0;
            #else
               shadow += current_depth - bias > pcf_depth ? 1.0 : 0.0;
            #endif
		   }
	   }
	   shadow /= count;
   #else
      float closest_depth = texture(shadow_map, proj_coord.xy).r; // distance from the light to the nearest surface along this angle
      float shadow;
      #if USE_INVERTED_SHADOW_MAP == 1
         shadow = current_depth + bias < closest_depth ? 1.0 : 0.0;
      #else
         shadow = current_depth - bias > closest_depth ? 1.0 : 0.0;
      #endif
   #endif
   //
   {
      float dist_x = (proj_coord.x - 0.5) / 0.5;
      float dist_y = (proj_coord.y - 0.5) / 0.5;
      float dist   = sqrt((dist_x * dist_x) + (dist_y * dist_y));
      //
      const float fade_start  = 0.8;
      const float fade_length = 1.0 - fade_start;
      dist -= fade_start;
      dist /= fade_length;
      shadow = max(shadow - max(dist, 0.0), 0.0);
   }
   //
   if (proj_coord.z > 1.0)
      //
      // Anything too far away for the light to "see" should be considered 
      // non-shadowed.
      //
      shadow = 0.0;
   return shadow;
}

#endif // include guard