#version 450

// receives three vertices; creates a triangle that fully covers the screen

void main() {
   // clockwise
   gl_Position = vec4(
      (gl_VertexIndex % 2) * 4.0 - 1.0, // -1 for most vertices; +3 for vertex 1
      (gl_VertexIndex & 2) * 2.0 - 1.0, // -1 for most vertices; +3 for vertex 2
      0.0,
      1.0
   );
}