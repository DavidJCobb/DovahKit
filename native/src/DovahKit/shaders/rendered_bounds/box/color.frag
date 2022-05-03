#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : enable

layout(location = 0) in VS_OUT {
   vec4 color;
} fs_in;

layout(location = 0) out vec4 out_color;

void main() {
   out_color = fs_in.color;
}