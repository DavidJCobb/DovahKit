#version 450

// receives three vertices; creates a triangle that fully covers the screen

void main() {
   // clockwise
   gl_Position = vec4(
      0.0,
      (gl_VertexIndex % 2) * 2.0, // vertex 1 only
      (gl_VertexIndex & 2),       // vertex 2 only
      1.0
   );
}