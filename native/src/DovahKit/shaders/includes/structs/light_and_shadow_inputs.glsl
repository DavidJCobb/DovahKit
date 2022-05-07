
#ifndef INCLUDE_GUARD_light_and_shadow_inputs
#define INCLUDE_GUARD_light_and_shadow_inputs

//
// A compact struct is needed because there is a (poorly specified) limit on 
// how much data can be passed from a vertex shader to a fragment shader. We 
// can query the physical device limits, but when we pass structs rather than 
// individual fields, we rely entirely on how the GPU lays those structs out, 
// which isn't specified by SPIR-V or Vulkan generally.
//
// You can use the "expand_light_and_shadow_inputs" helper function to unpack 
// this struct into the full "light_and_shadow_inputs" struct below.
//
struct compact_light_and_shadow_inputs {
   vec3 pos_world;
   //
   mat3 tangent_space;
   vec3 tangent_view_pos;
   vec4 sun_shadow_vert_pos;
   //
   // For active shadow-caster lights:
   //
   vec4 vector_to_light[4]; // XYZ is vector from pos_world to the shadow-caster light; W is the light distance ratio (distance / radius)
   vec3 light_space_pos[4];
};

struct light_and_shadow_inputs {
   vec3  pos_world;
   //
   mat3  tangent_space;
   vec3  tangent_sun_dir;
   vec3  tangent_view_pos;
   vec3  tangent_vert_pos;
   vec4  sun_shadow_vert_pos;
   //
   // For active shadow-caster lights:
   //
   vec3  vector_to_light[4];
   float light_distance_ratio[4];
   vec3  tangent_light_dir[4];
   vec3  light_space_pos[4];
};

#endif