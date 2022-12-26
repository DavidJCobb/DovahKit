#include "translate.h"
#include "../../raycast.h"
#include "helpers/math/geometry/ray_cone_intersection.h"
#include "helpers/math/geometry/ray_cylinder_intersection.h"

namespace vulkanDK::gizmos::meshes::translate {
   extern raycast_hit_data do_raycast(
      const glm::mat4& transform,
      const raycast& rc,
      //
      axis3D& out_which_axis
   ) {
      raycast_hit_data out = {};

      glm::vec3 origin = transform[3];

      auto test_stem = [&rc, &out_which_axis, &transform, &out, &origin](glm::vec3 start, axis3D which) {
         start = glm::vec4(start, 0) * transform;
         //
         float distance;
         if (cobb::geometry::ray_cylinder_intersection(
            rc.origin,
            rc.direction,
            start,
            origin,
            options::stem_radius,
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
         constexpr auto tip_pos = options::stem_length + options::arrowhead_length;

         auto base = glm::vec3(options::stem_length, options::stem_length, options::stem_length) * mult;
         auto tip  = glm::vec3(tip_pos, tip_pos, tip_pos) * mult;

         base = glm::vec4(base, 0) * transform;
         tip  = glm::vec4(tip,  0) * transform;

         float distance;
         if (cobb::geometry::ray_cone_intersection(
            rc.origin,
            rc.direction,
            tip,
            base,
            options::arrowhead_radius,
            false,
            distance
         )) {
            if (distance < out.distance) {
               out.distance   = distance;
               out_which_axis = which;
            }
         }
      };

      test_stem(glm::vec3(options::stem_length, 0, 0), axis3D::x);
      test_stem(glm::vec3(0, options::stem_length, 0), axis3D::y);
      test_stem(glm::vec3(0, 0, options::stem_length), axis3D::z);
      test_head(glm::vec3(1, 0, 0), axis3D::x);
      test_head(glm::vec3(0, 1, 0), axis3D::y);
      test_head(glm::vec3(0, 0, 1), axis3D::z);

      return out;
   }
}