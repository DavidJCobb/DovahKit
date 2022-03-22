struct scene_global_state {
   mat4 view;
   mat4 proj;
   vec3 ambient_light_color;
   vec3 sun_dir;
   vec3 sun_color;
	mat4 sun_space;
   //
   mat4 shadow_caster_proj_0;
   mat4 shadow_caster_proj_1;
   mat4 shadow_caster_proj_2;
   mat4 shadow_caster_proj_3;
   int  shadow_caster_index_0;
   int  shadow_caster_index_1;
   int  shadow_caster_index_2;
   int  shadow_caster_index_3;
};