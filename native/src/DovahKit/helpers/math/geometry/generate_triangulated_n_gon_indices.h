#pragma once
#include <vector>

namespace cobb::geometry {
   constexpr size_t triangle_indices_for_n_gon(size_t vertex_count) {
      return (vertex_count - 2) * 3;
   }

   //
   // Given a convex polygon with a given number of vertices, such as a disc, 
   // this function uses the "ear-shaving" method to triangulate the polygon, 
   // appending vertex indices to the passed-in vector. This function assumes 
   // that the vertices are ordered (either clockwise or counterclockwise), 
   // and will produce triangles with that same winding order.
   // 
   // "Ear-shaving" means that we start by building the smallest possible 
   // sliver polygons along the outer edge, essentially shaving the polygon 
   // down again and again until nothing remains.
   // 
   // The `vertex_offset` parameter is provided so that a more complex mesh 
   // which includes a convex N-gon (beginning at vertex X in the mesh's 
   // list) can have that N-gon triangulated using this function (pass X).
   //
   template<typename IndexContainer>
   constexpr void generate_triangulated_n_gon_indices(size_t vertex_count, size_t vertex_offset, IndexContainer& indices, size_t insert_indices_at) {
      size_t index = insert_indices_at;

      size_t layer = 1;
      for (; layer < vertex_count / 2; layer *= 2) {
         for (size_t v = 0; v < vertex_count; v += layer * 2) {
            size_t a = v;
            size_t b = v + (layer * 1);
            size_t c = v + (layer * 2);
            if (c > vertex_count)
               break;
            indices[index++] = (vertex_offset + a);
            indices[index++] = (vertex_offset + b % vertex_count);
            indices[index++] = (vertex_offset + c % vertex_count);
         }

         size_t last_tri_a = (vertex_count / (layer * 2)) * (layer * 2);
         size_t last_tri_b = last_tri_a + (layer * 1);
         size_t last_tri_c = last_tri_a + (layer * 2);
         if (last_tri_c > vertex_count && last_tri_b < vertex_count) {
            //
            // This layer has a hole.
            //
            indices[index++] = (vertex_offset + last_tri_a);
            indices[index++] = (vertex_offset + last_tri_b);
            indices[index++] = (vertex_offset + 0);
         }
      }
   }

   template<typename IndexType>
   constexpr void generate_triangulated_n_gon_indices(size_t vertex_count, size_t vertex_offset, std::vector<IndexType>& indices) {
      size_t start = indices.size();
      indices.resize(start + triangle_indices_for_n_gon(vertex_count));
      generate_triangulated_n_gon_indices(vertex_count, vertex_offset, indices, start);
   }
}