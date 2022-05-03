
#define DECLARE_ALL_TEXTURES_PARAMS \
   layout(set=SET_ALL_TEXTURES,binding=0) uniform sampler   default_sampler; \
   layout(set=SET_ALL_TEXTURES,binding=1) uniform texture2D textures[];