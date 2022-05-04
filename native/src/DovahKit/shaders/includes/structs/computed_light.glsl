
#ifndef INCLUDE_GUARD_computed_light // include guard
#define INCLUDE_GUARD_computed_light

struct computed_light {
   vec3 diffuse;  // light color and diffuse  strength; multiply the object's diffuse color into this
   vec3 specular; // light color and specular strength; multiply the object's specular strength and color into this
};

#endif // include guard