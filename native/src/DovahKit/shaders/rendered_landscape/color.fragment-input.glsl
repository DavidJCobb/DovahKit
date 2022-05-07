
#include "../includes/structs/light_and_shadow_inputs.glsl"

struct fragment_input {
   vec4  color;
   float blends[6];
   vec2  uv;
   //
   compact_light_and_shadow_inputs lighting_data;
};