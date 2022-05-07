
#if !defined(SET_SCENE_STATE)
   #error You included this file too early. Place it after the definition for the its index macro.
#endif

void apply_scene_fog(inout vec4 color, float camera_distance) {
   float range   = scene.fog_plane_far - scene.fog_plane_near;
   float coord   = clamp((camera_distance - scene.fog_plane_near) / range, 0.0F, 1.0F);
   float density = pow(coord, max(0.0F, scene.fog_power));
   density = min(min(1.0F, scene.fog_max), density);
   //
   vec3 fog_color = (scene.fog_color_far * (density)) + (scene.fog_color_near * (1.0F - density));
   color.rgb = mix(color.rgb, fog_color, density);
}