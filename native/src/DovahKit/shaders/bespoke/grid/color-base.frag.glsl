#extension GL_EXT_nonuniform_qualifier : require
#extension GL_OES_standard_derivatives : enable
#extension GL_GOOGLE_include_directive : enable

#include "../../includes/structs/scene_global_state.glsl"

#include "../../includes/descriptor_sets/scene_state.glsl"

#define SET_SCENE_STATE    0

DECLARE_SCENE_STATE_PARAMS    // scene

#include "color.fragment-input.glsl"
layout(location = 0) in VS_OUT {
   fragment_input fs_in;
};

#define COLOR_X vec3(1.0, 0.0, 0.0)
#define COLOR_Y vec3(0.1, 0.8, 0.0)

#define GRIDLINE_SPACING 64
#define ITERATIVE_GRIDLINES 0

vec4 calculate_color() {
   vec2 pos_xy = fs_in.pos_world.xy;
   #if ITERATIVE_GRIDLINES == 1
      int unit = int(max(abs(pos_xy.x), abs(pos_xy.y)));
      pos_xy /= GRIDLINE_SPACING;
      if (unit >= 256) {
         pos_xy /= 2;
         if (unit >= 256 + 512) {
            pos_xy /= 2;
         }
      }
   #else
      pos_xy /= GRIDLINE_SPACING;
   #endif

   vec2  grid = abs(fract(pos_xy - 0.5) - 0.5) / fwidth(pos_xy);
   //
   // abs(fract(n - 0.5) - 0.5) computes the distance from n to the nearest integer. 
   // This is, in the context of this shader, the offset from the nearest gridline. 
   // This value approaches zero as we get closer to a gridline.
   //
   // GPUs render fragments (i.e. pixels) in 2x2 chunks, and all calculations are 
   // essentially mirrored across them. Given some argument V, the fwidth function 
   // accesses the value of P in the diagonally opposite fragment, and subtracts 
   // that from P for the current fragment; the absolute value of the result is 
   // returned (so, it's always positive or zero, and can be thought of as the 
   // difference in magnitude across fragments).
   //
   // We take the fwidth of the XY-coordinates; this value approaches zero as we 
   // get further away on-screen (assuming perspective projection, of course). 
   // Using fwidth on world-relative positions allows us to scale calculations 
   // based on changes in camera distance over a short pixel distance; one use 
   // for this (built into the GPU and done for us) is mipmapping.
   //
   // Therefore, the result of dividing the former value by the latter will...
   //
   //    ...approach zero as we get closer to any gridline (we are dividing a 
   //       smaller value), or one as we get further from the nearest gridline.
   //
   //    ...approach zero as we get further away from the camera (we are dividing 
   //       by a larger value), or one as we get closer to the camera.
   //
   //    ...approach zero more quickly as we get both closer to the nearest grid-
   //       line and further from the camera.
   //
   //    ...approach one more quickly as we get both further from the nearest 
   //       gridline and closer to the camera.
   //
   // We subtract the result from 1.0 to invert it, and then we use it as a line 
   // brightness. This means that the line is brighter along gridlines, and its 
   // brightness falls off faster as it gets further from the camera; thus, a 
   // line with constant thickness and anti-aliasing.
   //
   float intensity_x = 1.0 - min(grid.x, 1.0);
   float intensity_y = 1.0 - min(grid.y, 1.0);
   
   float alpha = min(grid.x, grid.y);
   alpha = 1.0 - min(1.0, alpha);

   // Apply gamma correction
   intensity_x = pow(intensity_x, 1.0 / 2.2);
   intensity_y = pow(intensity_y, 1.0 / 2.2);

   vec4 color = vec4(COLOR_X * intensity_x + COLOR_Y * intensity_y, alpha);
   if (!gl_FrontFacing) {
      //
      // Darken the grid slightly if viewed from below (i.e. from the back).
      //
      color.rgb *= 0.4;
   }
   return color;
}