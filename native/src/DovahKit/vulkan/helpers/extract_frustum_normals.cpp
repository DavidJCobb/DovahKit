#include "extract_frustum_normals.h"

namespace {
   template<size_t axis, bool sub> void _make_normal(const glm::mat4& view_proj, glm::vec4& out) {
      for (int i = 0; i < 4; ++i) {
         float v;
         if constexpr (axis == 0)
            v = view_proj[i].x;
         else if constexpr (axis == 1)
            v = view_proj[i].y;
         else if constexpr (axis == 2)
            v = view_proj[i].z;
         //
         if constexpr (sub)
            out[i] = view_proj[i].w - v;
         else
            out[i] = view_proj[i].w + v;
      }
   }
}

namespace vulkanDK {
   extern std::array<glm::vec4, 4> extract_frustum_normals(const glm::mat4& view_proj) {
      std::array<glm::vec4, 4> out;
      _make_normal<0, false>(view_proj, out[0]); // left   plane
      _make_normal<0, true>(view_proj, out[1]);  // right  plane
      _make_normal<1, false>(view_proj, out[2]); // top    plane
      _make_normal<1, true>(view_proj, out[3]);  // bottom plane
      return out;
   }
}