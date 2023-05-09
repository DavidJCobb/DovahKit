#include "rotate.h"
#include "helpers/math/cosine.h"
#include "helpers/math/sine.h"
#include "helpers/unreachable.h"
#include "../../raycast.h"
#include "helpers/math/geometry/ray_cylinder_intersection.h"

namespace {
   auto _nth_vertex = [](int axis, size_t n) -> glm::vec3 {
      using namespace vulkanDK::gizmos::meshes::rotate;

      constexpr float radians_per_loop = (2 * std::numbers::pi_v<float>) / (float)options::loops_per_hoop;

      float rel_x = impl::hoop_center_radius * cobb::cosine(n * radians_per_loop);
      float rel_y = impl::hoop_center_radius * cobb::sine(n * radians_per_loop);
      switch (axis) {
         case 0: return { 0, rel_x, rel_y }; break;
         case 1: return { rel_x, 0, rel_y }; break;
         case 2: return { rel_x, rel_y, 0 }; break;
      }
   };

   auto _axis_vertices = [](vulkanDK::axis3D axis) {
      using namespace vulkanDK::gizmos::meshes::rotate;

      std::array<glm::vec3, options::loops_per_hoop> list;

      constexpr float radians_per_loop = (2 * std::numbers::pi_v<float>) / (float)options::loops_per_hoop;
      for (size_t i = 0; i < options::loops_per_hoop; ++i) {
         list[i] = { 0, 0, 0 };

         float rel_x = impl::hoop_center_radius * cobb::cosine(i * radians_per_loop);
         float rel_y = impl::hoop_center_radius * cobb::sine(i * radians_per_loop);
         switch (axis) {
            using enum vulkanDK::axis3D;
            case x: list[i] = { 0, rel_x, rel_y }; break;
            case y: list[i] = { rel_x, 0, rel_y }; break;
            case z: list[i] = { rel_x, rel_y, 0 }; break;
         }
      }

      return list;
   };

   constexpr const auto _vertices_per_axis = []() {
      using namespace vulkanDK::gizmos::meshes::rotate;
      using list_type = std::array<glm::vec3, options::loops_per_hoop>;

      struct {
         list_type x = _axis_vertices(vulkanDK::axis3D::x);
         list_type y = _axis_vertices(vulkanDK::axis3D::y);
         list_type z = _axis_vertices(vulkanDK::axis3D::z);
      } value;
      return value;
   }();
   constexpr const auto& _vertices_for_axis(vulkanDK::axis3D which) {
      switch (which) {
         using enum vulkanDK::axis3D;
         case x: return _vertices_per_axis.x;
         case y: return _vertices_per_axis.y;
         case z: return _vertices_per_axis.z;
      }
      cobb::unreachable();
   }
}

namespace vulkanDK::gizmos::meshes::rotate {
   extern raycast_hit_data do_raycast(
      const glm::mat4& transform,
      const raycast& rc,
      //
      axis3D& out_which_axis
   ) {
      raycast_hit_data out = {};

      constexpr auto verts_x = _axis_vertices(axis3D::x);
      constexpr auto verts_y = _axis_vertices(axis3D::y);
      constexpr auto verts_z = _axis_vertices(axis3D::z);

      auto _test_loop = [&rc, &transform, &out, &out_which_axis](axis3D which) {
         auto& list = _vertices_for_axis(which);
         for (size_t i = 0; i < options::loops_per_hoop; ++i) {
            const auto& cap_a_local = list[i];
            const auto& cap_b_local = list[(i + 1) % list.size()];

            glm::vec3 cap_a = glm::vec4(cap_a_local, 0) * transform;
            glm::vec3 cap_b = glm::vec4(cap_b_local, 0) * transform;

            float distance;
            if (cobb::geometry::ray_cylinder_intersection(
               rc.origin,
               rc.direction,
               cap_a,
               cap_b,
               options::hoop_thickness / 2,
               false,
               distance
            )) {
               if (distance < out.distance) {
                  out.distance = distance;
                  out_which_axis = which;
               }
            }
         }
      };

      _test_loop(axis3D::x);
      _test_loop(axis3D::y);
      _test_loop(axis3D::z);

      return out;
   }
}