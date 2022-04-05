
#ifndef included_rendered_light_shader_params // include guard
#define included_rendered_light_shader_params

struct rendered_light_shader_params {
   mat4  transform;
   mat4  transform_inv;
   vec3  color;
   float fade;
   float falloff;
   float fov;
   float radius;
   int   type; // rendered_light::light_type enum in C++
};

#define RENDERED_LIGHT_TYPE_OMNI        0
#define RENDERED_LIGHT_TYPE_OMNI_SHADOW 1
#define RENDERED_LIGHT_TYPE_HEMI_SHADOW 2
#define RENDERED_LIGHT_TYPE_SPOT_SHADOW 3

bool rendered_light_can_cast_shadows(rendered_light_shader_params l) {
   switch (l.type) {
      case RENDERED_LIGHT_TYPE_OMNI_SHADOW:
      case RENDERED_LIGHT_TYPE_HEMI_SHADOW:
      case RENDERED_LIGHT_TYPE_SPOT_SHADOW:
         return true;
   }
   return false;
}

#endif // include guard