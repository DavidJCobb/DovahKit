#pragma once
#include "../_vulkan.h"
#include "../buffer.h"

namespace vulkanDK {
   class command_buffer;
   class surface_renderer;
}

namespace vulkanDK::gizmos {
   // Vertex-and-index buffer, and associated information, for the edit gizmo's 
   // models.
   struct vib {
      struct mesh_info {
         size_t first_index  = 0; // index of first index in this mesh
         size_t index_count  = 0; // number of indices in this mesh
         size_t first_vertex = 0; // index of first vertex in this mesh
         size_t vertex_count = 0; // number of vertices in this mesh
      };

      buffer data;
      size_t indices_start  = 0; // byte-offset of all indices; indices for all mesh are stored contiguously after vertices for all meshes
      size_t vertices_start = 0; // byte-offset of all vertices
      struct {
         mesh_info translate;
         mesh_info rotate;
         mesh_info scale;
      } meshes;

      void setup(surface_renderer&);

      void record_draw(surface_renderer&, command_buffer&);
   };
}
