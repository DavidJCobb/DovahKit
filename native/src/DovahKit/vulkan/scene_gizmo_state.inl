#pragma once
#include "helpers/as_const.h"
#include "helpers/unreachable.h"
#include "scene_gizmo_state.h"

namespace vulkanDK {
   constexpr gizmo_mode scene_gizmo_state::get_mode() const noexcept {
      if (this->flags & flag::mode_translate)
         return gizmo_mode::translate;
      if (this->flags & flag::mode_rotate)
         return gizmo_mode::rotate;
      if (this->flags & flag::mode_scale)
         return gizmo_mode::scale;
      return gizmo_mode::none;
   }
   constexpr void scene_gizmo_state::set_mode(gizmo_mode gm) noexcept {
      this->flags &= ~flag::all_modes;
      switch (gm) {
         using enum gizmo_mode;
         case translate: this->flags |= flag::mode_translate; break;
         case rotate:    this->flags |= flag::mode_rotate;    break;
         case scale:     this->flags |= flag::mode_scale;     break;
      }
   }

   constexpr bool scene_gizmo_state::is_axis_highlighted(axis3D a) const noexcept {
      switch (a) {
         case axis3D::x: return (this->flags & flag::highlight_axis_x) != 0;
         case axis3D::y: return (this->flags & flag::highlight_axis_y) != 0;
         case axis3D::z: return (this->flags & flag::highlight_axis_z) != 0;
      }
      return false;
   }
   constexpr void scene_gizmo_state::set_axis_highlighted(axis3D a, bool v) noexcept {
      flags_t flag = 0;
      switch (a) {
         case axis3D::x: flag = flag::highlight_axis_x; break;
         case axis3D::y: flag = flag::highlight_axis_y; break;
         case axis3D::z: flag = flag::highlight_axis_z; break;
      }
      if (v) {
         this->flags |= flag;
      } else {
         this->flags &= ~flag;
      }
   }

   constexpr const glm::vec3& scene_gizmo_state::axis_color(axis3D a) const noexcept {
      switch (a) {
         case axis3D::x: return this->color_x;
         case axis3D::y: return this->color_y;
         case axis3D::z: return this->color_z;
      }
      cobb::unreachable();
   }
   constexpr glm::vec3& scene_gizmo_state::axis_color(axis3D a) noexcept {
      return const_cast<glm::vec3&>(cobb::as_const(this)->axis_color(a));
   }
}