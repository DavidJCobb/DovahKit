#version 460

layout(input_attachment_index = 0, binding = 0) uniform subpassInput map_accumulator;
layout(input_attachment_index = 1, binding = 1) uniform subpassInput map_reveal;

layout(location = 0) out vec4 out_color;

void main() {
   vec4  accum = subpassLoad(map_accumulator);
   float alpha = subpassLoad(map_reveal).r;

   // Blend:
   // src: ONE_MINUS_SRC_ALPHA
   // dst: SRC_ALPHA
   out_color = vec4(accum.rgb / max(accum.a, 0.00001), alpha);
}