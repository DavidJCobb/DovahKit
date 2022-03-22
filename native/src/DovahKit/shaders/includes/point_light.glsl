
struct point_light {
   mat4  transform;
   vec3  color;
   float radius;
   float fade;
   int   type; // rendered_light::light_type enum in C++
};