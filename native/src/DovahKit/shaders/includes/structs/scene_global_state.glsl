
#ifndef INCLUDE_GUARD_scene_global_state
#define INCLUDE_GUARD_scene_global_state

#extension GL_EXT_scalar_block_layout : require

struct scene_global_state {
   mat4 view;
   mat4 proj;
   vec3 camera_pos;
   int  flags;
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
   //
   vec3 landscape_border_color_a;
   int  default_land_diffuse_texture;
   vec3 landscape_border_color_b;
   int  default_land_normals_texture;
};

#define SCENE_GLOBAL_STATE_FLAG_SHOW_LANDSCAPE_BORDERS 0x00000001

#endif