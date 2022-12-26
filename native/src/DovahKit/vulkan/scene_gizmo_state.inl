#pragma once
#include "helpers/as_const.h"
#include "helpers/unreachable.h"
#include "scene_gizmo_state.h"

namespace vulkanDK {
   static_assert((int)gizmo_mode::translate == 1, "If the underlying values for this enum have changed, then the shaders and the code here need updating.");
   static_assert((int)gizmo_mode::rotate    == 2, "If the underlying values for this enum have changed, then the shaders and the code here need updating.");
   static_assert((int)gizmo_mode::scale     == 3, "If the underlying values for this enum have changed, then the shaders and the code here need updating.");

   constexpr gizmo_mode scene_gizmo_state::get_mode() const noexcept {
      return (gizmo_mode)(this->flags & flag::reserved_bits_mode);
   }
   constexpr void scene_gizmo_state::set_mode(gizmo_mode gm) noexcept {
      this->flags &= ~flag::reserved_bits_mode;
      this->flags |= ((uint32_t)gm) & flag::reserved_bits_mode;
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
   constexpr void scene_gizmo_state::replace_axis_highlighted(axis3D a) noexcept {
      this->flags &= ~(flag::all_highlight);
      this->set_axis_highlighted(a, true);
   }
   constexpr void scene_gizmo_state::clear_all_axis_highlighting() noexcept {
      this->flags &= ~(flag::all_highlight);
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