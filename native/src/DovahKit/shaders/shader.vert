#version 450

//
// Location sizes:
//
//  - most types | 1
//  - double     | 1
//  - dvec2      | 1
//  - dvec3      | 2
//  - dvec4      | 2
//
// Care must be taken to ensure that parameters don't overlap... unless 
// they're meant to.
//

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() {
   gl_Position = vec4(inPosition, 0.0, 1.0);
   fragColor = inColor;
}