#pragma once
#include "./range_control_conflict_state.h"

namespace dovahkit::subsystems::worldinput::util {
   constexpr bool range_control_conflict_state::is_axis_blocked(axis2D axis) const {
      return (this->blocked_axes >> (int)axis) & 1;
   }
   constexpr void range_control_conflict_state::set_axis_blocked(axis2D axis, bool state) {
      auto mask = 1 << (int)axis;
      if (state)
         this->blocked_axes |= mask;
      else
         this->blocked_axes &= ~mask;
   }

   constexpr bool range_control_conflict_state::are_axes_blocked(range_input_axes axes) const {
      if (this->blocked_axes == all_valid_axes_mask)
         return true;

      uint8_t mask = all_valid_axes_mask;
      switch (axes) {
         case range_input_axes::x:
            mask = 1 << (int)axis2D::x;
            break;
         case range_input_axes::y:
            mask = 1 << (int)axis2D::y;
            break;
         case range_input_axes::all:
            mask = all_valid_axes_mask;
            break;
      }
      return (this->blocked_axes & mask) == mask;
   }
   constexpr void range_control_conflict_state::set_axes_blocked(range_input_axes axes, bool state) {
      uint8_t mask = 0;
      switch (axes) {
         case range_input_axes::x:
            mask = 1 << (int)axis2D::x;
            break;
         case range_input_axes::y:
            mask = 1 << (int)axis2D::y;
            break;
         case range_input_axes::all:
            mask = all_valid_axes_mask;
            break;
      }
      if (state)
         this->blocked_axes |= mask;
      else
         this->blocked_axes &= ~mask;
   }

   constexpr bool range_control_conflict_state::are_all_axes_blocked() const {
      return (this->blocked_axes & all_valid_axes_mask) == all_valid_axes_mask;
   }
   constexpr void range_control_conflict_state::set_all_axes_blocked() {
      this->blocked_axes = all_valid_axes_mask;
   }

   constexpr range_control_conflict_state& range_control_conflict_state::operator&=(const range_control_conflict_state& other) noexcept {
      this->blocked_axes &= other.blocked_axes;
      return *this;
   }
   constexpr range_control_conflict_state range_control_conflict_state::operator&(const range_control_conflict_state& other) const noexcept {
      return (range_control_conflict_state(*this) &= other);
   }

   constexpr range_control_conflict_state& range_control_conflict_state::operator|=(const range_control_conflict_state& other) noexcept {
      this->blocked_axes |= other.blocked_axes;
      return *this;
   }
   constexpr range_control_conflict_state range_control_conflict_state::operator|(const range_control_conflict_state& other) const noexcept {
      return (range_control_conflict_state(*this) |= other);
   }

   constexpr void range_control_conflict_state::constrain_value(QPointF& value) {
      if (this->blocked_axes == 0)
         return;
      if (this->blocked_axes == all_valid_axes_mask) {
         value = {};
         return;
      }

      if (this->is_axis_blocked(axis2D::x))
         value.rx() = 0;
      if (this->is_axis_blocked(axis2D::y))
         value.ry() = 0;
   }
}