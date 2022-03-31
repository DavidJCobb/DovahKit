
#ifndef included_point_light // include guard
#define included_point_light

struct point_light {
   mat4  transform;
   vec3  color;
   float radius;
   float fade;
   int   type; // rendered_light::light_type enum in C++
};

bool point_light_can_cast_shadows(point_light l) {
   switch (l.type) {
      case 1: // omni_shadow
      case 2: // hemi_shadow
      case 3: // spot_shadow
         return true;
   }
   return false;
}

#endif // include guard