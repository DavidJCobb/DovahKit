
#define DECLARE_SHADOW_MAPS_PARAMS \
   layout(set=SET_SHADOW_MAPS,binding=0) uniform sampler2D   sun_shadow_map;     \
   layout(set=SET_SHADOW_MAPS,binding=1) uniform samplerCube light_shadow_maps[];