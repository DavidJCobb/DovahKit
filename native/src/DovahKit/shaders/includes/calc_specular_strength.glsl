
#define BLINN_PHONG_MODE_PHONG 0
#define BLINN_PHONG_MODE_BLINN 1
//
#define BLINN_PHONG_MODE BLINN_PHONG_MODE_BLINN

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