#include "move_selection_by_drag.h"
#include "../options_union.h"
#include "../tool_response_tuple.h"

#include "helpers/math/geometry/ray_plane_intersection.h"
#include "../../core.h"

namespace dovahkit::subsystems::worldedit::tools {
   /*static*/ void move_selection_by_drag::request(const tool_request_cause& input, const opaque_options_union& raw_options, tool_response_tuple& all_results) {
      const options& o = raw_options.as<options>();

      if (!input.raycast.has_value())
         return;
      if (!input.raycast.value().hit_position.has_value())
         return;
      
      if (input.pointer.delta.x() == 0 && input.pointer.delta.y() == 0)
         return;

      auto& worldedit_core = core::get();
      auto  camera_matrix  = worldedit_core.get_frame_rotation_matrix(reference_frame::camera);

      glm::fvec3 grab_point = input.raycast.value().hit_position.value();

      qDebug(" - move_selection_by_drag: cursor movement (%dpx, %dpx)", input.pointer.delta.x(), input.pointer.delta.y());
      qDebug("    - grab point is (%f, %f, %f)", grab_point.x, grab_point.y, grab_point.z);

      //
      // Convert the screen delta -- the pointer movement in pixels -- into a pointer movement 
      // in world units.
      //
      glm::fvec3 world_delta;
      {
         glm::fvec3 ray_prior_origin;
         glm::fvec3 ray_prior_direction;
         glm::fvec3 ray_after_origin;
         glm::fvec3 ray_after_direction;
         {
            auto pointer_pos_now  = input.pointer.pos;
            auto pointer_pos_prev = pointer_pos_now - input.pointer.delta;

            qDebug("    - cursor moved from (%d, %d) to (%d, %d)", pointer_pos_prev.x(), pointer_pos_prev.y(), pointer_pos_now.x(), pointer_pos_now.y());

            bool renderer_available;
            //
            renderer_available = worldedit_core.get_raycast_vectors(
               pointer_pos_prev.x(),
               pointer_pos_prev.y(),
               ray_prior_origin,
               ray_prior_direction
            );
            renderer_available &= worldedit_core.get_raycast_vectors(
               pointer_pos_now.x(),
               pointer_pos_now.y(),
               ray_after_origin,
               ray_after_direction
            );
            //
            if (!renderer_available)
               return;
         }

         //
         // To convert the pointer movement from screen pixels to world units, we need to perform 
         // ray/plane intersection tests between two rays (representing the pointer screen positions 
         // before and after the pointer movement) and a plane. We can then subtract the "before" hit 
         // position from the "after" hit position to get the movement in world units.
         // 
         // If our goal is to drag-move the selection along a plane, then we can use that plane as the 
         // "space" we convert pointer movement to (i.e. we're projecting the "screen plane" onto the 
         // desired movement plane), and the resulting movement vector can be applied directly to the 
         // selection.
         // 
         // If our goal is to drag-move the selection along an axis, then the plane we want to use will 
         // be parallel to the "screen plane." Once we have the movement in world units, we can then 
         // project it onto the 3D axis we want to drag along, to flatten it from 2D-in-3D to 1D-in-3D, 
         // and the resulting movement vector can then be applied directly to the selection.
         //
         glm::fvec3 plane_normal;
         if (!o.is_plane) {
            //
            // If we're dragging along an axis, then always use the screen plane.
            //
            plane_normal = camera_matrix[1];
         } else {
            glm::mat3 frame_matrix;
            if (o.frame == reference_frame::camera) {
               frame_matrix = camera_matrix;
            } else {
               frame_matrix = worldedit_core.get_frame_rotation_matrix(o.frame);
            }

            switch (o.axis) { // plane normal
               case axis3D::x:
                  plane_normal = frame_matrix[0];
                  break;
               case axis3D::y:
                  plane_normal = frame_matrix[1];
                  break;
               case axis3D::z:
                  plane_normal = frame_matrix[2];
                  break;
            }
         }
         plane_normal = glm::normalize(plane_normal); // proofing, in case the camera ever scales or gets FP inaccuracy somehow

         //
         // Now that we've extracted a plane definition, we should run the ray/plane intersection 
         // tests.
         // 

         qDebug("    - 2D-in-2D to 2D-in-3D:");
         qDebug("       - plane normal is (%f, %f, %f)", plane_normal.x, plane_normal.y, plane_normal.z);
         qDebug("       - ray origin (prior) is (%f, %f, %f)", ray_prior_origin.x, ray_prior_origin.y, ray_prior_origin.z);
         qDebug("       - ray origin (after) is (%f, %f, %f)", ray_after_origin.x, ray_after_origin.y, ray_after_origin.z);
         {
            auto _debug_delta = ray_after_origin - ray_prior_origin;
            qDebug("          - delta: (%f, %f, %f)", _debug_delta.x, _debug_delta.y, _debug_delta.z);
         }
         qDebug("       - ray directions are (%f, %f, %f)", ray_prior_direction.x, ray_prior_direction.y, ray_prior_direction.z); // both directions should be essentially the same

         glm::fvec3 hit_prior;
         {
            float distance;
            bool  intersects = cobb::geometry::ray_plane_intersection(
               ray_prior_origin,
               ray_prior_direction,
               grab_point,
               plane_normal,
               distance
            );
            if (!intersects) // should never happen
               return;
            hit_prior = ray_prior_origin + (ray_prior_direction * distance);
         }
         
         glm::fvec3 hit_after;
         {
            float distance;
            bool  intersects = cobb::geometry::ray_plane_intersection(
               ray_after_origin,
               ray_after_direction,
               grab_point,
               plane_normal,
               distance
            );
            if (!intersects) // should never happen
               return;
            hit_after = ray_after_origin + (ray_after_direction * distance);
         }

         world_delta = hit_after - hit_prior;

         qDebug("       - world delta is (%f, %f, %f)", world_delta.x, world_delta.y, world_delta.z);
      }

      //
      // We've finished converting the pointer movement from screen-space to world-space.
      //
      // If we only want to move along a single axis, then flatten the pointer-movement-in-
      // world-units onto that axis.
      //
      if (!o.is_plane) {
         glm::fvec3 drag_axis;
         {
            glm::mat3 frame_matrix;
            if (o.frame == reference_frame::camera) {
               frame_matrix = camera_matrix;
            } else {
               frame_matrix = worldedit_core.get_frame_rotation_matrix(o.frame);
            }
            switch (o.axis) {
               case axis3D::x: drag_axis = frame_matrix[0]; break;
               case axis3D::y: drag_axis = frame_matrix[1]; break;
               case axis3D::z: drag_axis = frame_matrix[2]; break;
            }
         }

         world_delta = drag_axis * glm::dot(world_delta, drag_axis);
      }

      qDebug("    - final movement is (%f, %f, %f)", world_delta.x, world_delta.y, world_delta.z);

      //
      // The movement vector is now in a form suitable to be applied as a translation of 
      // the selected entities.
      //

      response res;
      res.translate_by = world_delta;
      all_results.merge_member(input, res); // TODO: nothing; this will stop being an error once this tool is added to the usual class-arrays for tools
   }
   /*static*/ void move_selection_by_drag::request_for_hold_release(const opaque_options_union&, tool_response_tuple&) {
      // No-op.
   }
}

namespace dovahkit::subsystems::worldedit::tools {
   /*static*/ void move_selection_by_drag::invoke(const response& params) {
      auto& worldedit_core = core::get();
      
      auto adjust = core::coordinate_adjustment{
         .rotate = {
            .euler = {},
            .frame = reference_frame::world,
         },
         .translate = params.translate_by,
      };
      worldedit_core.try_adjust_selection_coordinates(adjust);
   }
}