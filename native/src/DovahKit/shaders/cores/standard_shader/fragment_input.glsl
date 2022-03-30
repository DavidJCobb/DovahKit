
struct fragment_input {
   vec4  color; // vertex color
   vec2  uv;
   vec3  pos_world;
   mat3  tangent_space;
   vec3  tangent_sun_dir;
   vec3  tangent_view_pos;
   vec3  tangent_vert_pos;
   vec4  sun_shadow_vert_pos;
   float camera_distance;
   //
   vec3  vector_to_light[4];
   float light_distance_ratio[4];
   vec3  tangent_light_dir[4];
};