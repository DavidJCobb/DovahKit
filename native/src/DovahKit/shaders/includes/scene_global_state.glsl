struct scene_global_state {
   mat4 view;
   mat4 proj;
   vec3 ambient_light_color;
   vec3 sun_dir;
   vec3 sun_color;
	mat4 sun_space;
   //
   int  shadow_caster_index[4];
   //
   vec3  fog_color_near;
   float fog_plane_near;
   vec3  fog_color_far;
   float fog_plane_far;
   float fog_power;
   float fog_max;
   float interior_clip_distance;
};