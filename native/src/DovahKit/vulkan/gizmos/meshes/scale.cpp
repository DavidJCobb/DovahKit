#include "scale.h"
#include "../../raycast.h"
#include "helpers/math/geometry/ray_OBB_intersection.h"
#include "helpers/math/geometry/ray_cylinder_intersection.h"
#include "helpers/math/sqrt.h"
#include "helpers/unreachable.h"

namespace vulkanDK::gizmos::meshes::scale {
   extern raycast_hit_data do_raycast(
      const glm::mat4& transform,
      const raycast& rc,
      //
      axis3D& out_which_axis
   ) {
      raycast_hit_data out = {};

      glm::vec3 origin = transform[3];

      //
      // Our stem is a right-triangular prism, but we're doing a ray/cylinder test. In 2D terms, 
      // we want to test our ray against the triangle's circumscribed circle -- the smallest 
      // circle that wholly contains the triangle.
      // 
      // The hypotenuse of the triangle is this circle's diameter, and the circumcenter -- the 
      // centerpoint of the circle -- is the hypotenuse's midpoint.
      //
      constexpr const float stem_circumradius = cobb::sqrt((options::stem_radius * 2) * (options::stem_radius * 2) * 2) / 2 + options::raycast_inflation;
      constexpr const float stem_circumcenter = options::stem_radius;

      auto test_stem = [&rc, &out_which_axis, &transform, &out, &origin](glm::vec3 start, axis3D which) {
         start = transform * glm::vec4(start, 1);
         //
         float distance;
         if (cobb::geometry::ray_cylinder_intersection(
            rc.origin,
            rc.direction,
            start,
            origin,
            stem_circumradius,
            false,
            distance
         )) {
            if (distance < out.distance) {
               out.distance   = distance;
               out_which_axis = which;
            }
         }
      };
      auto test_head = [&rc, &out_which_axis, &transform, &out](glm::vec3 mult, axis3D which) {
         constexpr auto aabb_min = glm::vec3(-1, -1, -1) * ((options::handle_width + options::raycast_inflation) / 2);
         constexpr auto aabb_max = glm::vec3( 1,  1,  1) * ((options::handle_width + options::raycast_inflation) / 2);

         constexpr auto box_center_distance = options::stem_length + (options::handle_width / 2);

         glm::mat4 box_transform = transform;
         {
            const auto& box_axis = [&transform, which]() {
               switch (which) {
                  case axis3D::x: return transform[0];
                  case axis3D::y: return transform[1];
                  case axis3D::z: return transform[2];
               }
               cobb::unreachable();
            }();
            box_transform[3] += box_axis * box_center_distance;
         }

         float distance;
         if (cobb::geometry::ray_OBB_intersection(
            rc.origin,
            rc.direction,
            aabb_min,
            aabb_max,
            box_transform,
            false,
            distance
         )) {
            if (distance < out.distance) {
               out.distance = distance;
               out_which_axis = which;
            }
         }
      };

      test_stem(glm::vec3(options::stem_length, stem_circumcenter, stem_circumcenter), axis3D::x);
      test_stem(glm::vec3(stem_circumcenter, options::stem_length, stem_circumcenter), axis3D::y);
      test_stem(glm::vec3(stem_circumcenter, stem_circumcenter, options::stem_length), axis3D::z);
      test_head(glm::vec3(1, 0, 0), axis3D::x);
      test_head(glm::vec3(0, 1, 0), axis3D::y);
      test_head(glm::vec3(0, 0, 1), axis3D::z);

      if (out) {
         out.position = rc.origin + (rc.direction * out.distance);
      }
      return out;
   }
}