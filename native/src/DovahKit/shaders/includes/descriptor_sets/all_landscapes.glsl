
#include "../structs/rendered_landscape_shader_params.glsl"

#define DECLARE_ALL_LANDSCAPES_PARAMS \
   layout(std430,set=SET_ALL_LANDSCAPES,binding=0) readonly buffer AllLandscapes { rendered_landscape_shader_params landscapes[]; };