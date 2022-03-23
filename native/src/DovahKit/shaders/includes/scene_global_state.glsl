struct scene_global_state {
   mat4 view;
   mat4 proj;
   vec3 ambient_light_color;
   vec3 sun_dir;
   vec3 sun_color;
	mat4 sun_space;
   //
   mat4 shadow_caster_space_pos[4];
   mat4 shadow_caster_space_neg[4];
   int  shadow_caster_index[4];
};