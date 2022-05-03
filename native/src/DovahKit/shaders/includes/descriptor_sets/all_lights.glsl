
#define DECLARE_ALL_LIGHTS_PARAMS \
	layout(std140,set=SET_ALL_LIGHTS,binding=0) readonly buffer AllLights { rendered_light_shader_params scene_lights[]; };