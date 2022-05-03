
#define DECLARE_SHADOW_CASTER_MAP_RENDER_PARAMS \
	layout(std430,set=SET_SHADOW_CASTER_MAP_RENDER,binding = 0) readonly buffer LightViewProjMatrices { mat4 light_space_matrices[4][6]; };