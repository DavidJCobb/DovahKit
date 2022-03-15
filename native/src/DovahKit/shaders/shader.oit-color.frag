#version 460
#extension GL_GOOGLE_include_directive : enable

#define USE_ALPHA 1
#include "cores/standard_shader.frag"

layout(location = 0) out vec4  out_accumulator;
layout(location = 1) out float out_reveal;

float compute_oit_weight(vec4 color) {
   // scale the camera-relative depth to range [0.01, 500]
   const float min_depth =    0.01;
   const float max_depth = 4096.0;
   const float scaled_depth = (fs_in.camera_distance - min_depth) / (max_depth - min_depth) * (500.0 - 0.01) + 0.01;
   
   // equation 9
   float distance_weight = clamp(
      0.03 / (0.00001 + pow(scaled_depth / 5.0, 4)),
      0.01,
      3000
   );
   
   return distance_weight * color.a;
}

void main() {
   vec4 color = calculate_color();
   color.rgb *= color.a; // premultiply alpha
   
   const float weight = compute_oit_weight(color);
   
   out_accumulator = color * weight; // blend: src VK_BLEND_FACTOR_ONE,  dst VK_BLEND_FACTOR_ONE
   out_reveal      = color.a;        // blend: src VK_BLEND_FACTOR_ZERO, dst VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
}