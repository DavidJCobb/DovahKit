
#ifndef included_point_light // include guard
#define included_point_light

struct point_light {
   mat4  transform;
   mat4  transform_inv;
   vec3  color;
   float radius;
   float fade;
   int   type; // rendered_light::light_type enum in C++
};

#define RENDERED_LIGHT_TYPE_OMNI        0
#define RENDERED_LIGHT_TYPE_OMNI_SHADOW 1
#define RENDERED_LIGHT_TYPE_HEMI_SHADOW 2
#define RENDERED_LIGHT_TYPE_SPOT_SHADOW 3

bool point_light_can_cast_shadows(point_light l) {
   switch (l.type) {
      case RENDERED_LIGHT_TYPE_OMNI_SHADOW:
      case RENDERED_LIGHT_TYPE_HEMI_SHADOW:
      case RENDERED_LIGHT_TYPE_SPOT_SHADOW:
         return true;
   }
   return false;
}

#endif // include guard