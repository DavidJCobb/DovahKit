
#ifndef INCLUDE_GUARD_scene_gizmo_state
#define INCLUDE_GUARD_scene_gizmo_state

#extension GL_EXT_scalar_block_layout : require

struct scene_gizmo_state {
   vec4 color_highlight;
   vec3 color_x;
   int  flags;
   vec3 color_y;
   int  padding_1;
   vec3 color_z;
   int  padding_2;
   mat4 transform;
};

#define SCENE_GIZMO_STATE_FLAG_BITS_MODE 3
//
// (flags & SCENE_GIZMO_STATE_FLAG_BITS_MODE) == one of the below:
//
#define SCENE_GIZMO_STATE_MODE_NONE      0
#define SCENE_GIZMO_STATE_MODE_TRANSLATE 1
#define SCENE_GIZMO_STATE_MODE_ROTATE    2
#define SCENE_GIZMO_STATE_MODE_SCALE     3

#define SCENE_GIZMO_STATE_FLAG_HIGHLIGHT_AXIS_X 0x00000010
#define SCENE_GIZMO_STATE_FLAG_HIGHLIGHT_AXIS_Y 0x00000020
#define SCENE_GIZMO_STATE_FLAG_HIGHLIGHT_AXIS_Z 0x00000040

#endif