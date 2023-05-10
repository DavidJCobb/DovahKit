#pragma once
#include <optional>
#include <QPointF>
#include <glm/glm.hpp>
#include "vulkan/enums/gizmo_mode.h"
#include "./enums/axis3D.h"

namespace dovah {
   class form_stub;
}

namespace dovahkit::subsystems::worldedit {
   struct raycast_result {
      public:
         using axis3D     = worldedit::axis3D;
         using gizmo_mode = vulkanDK::gizmo_mode;

      public:
         std::optional<glm::vec3> hit_position;
         QPointF view_position;
         struct {
            struct {
               axis3D     axis = axis3D::x;
               gizmo_mode mode = gizmo_mode::none;
            } edit_gizmo;
            dovah::form_stub* form = nullptr;

            bool is_selected = false;
         } target_info;

         constexpr bool empty() const noexcept {
            return !this->hit_position.has_value();
         }

         constexpr bool same_target_as(const raycast_result& other) const noexcept {
            const auto& a = this->target_info;
            const auto& b = other.target_info;
            if (a.edit_gizmo.mode != b.edit_gizmo.mode)
               return false;
            if (a.edit_gizmo.mode != gizmo_mode::none) {
               if (a.edit_gizmo.axis != b.edit_gizmo.axis)
                  return false;
            }
            if (a.form != b.form)
               return false;
            if (a.is_selected != b.is_selected)
               return false;

            return true;
         }

         constexpr bool operator==(const raycast_result& other) const noexcept {
            if (this->hit_position != other.hit_position)
               return false;
            if (this->view_position != other.view_position)
               return false;
            if (!this->same_target_as(other))
               return false;
            return true;
         }
   };
}