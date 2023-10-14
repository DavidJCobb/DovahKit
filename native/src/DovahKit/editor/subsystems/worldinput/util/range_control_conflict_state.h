#pragma once
#include <cstdint>
#include <QPointF>
#include "../enums/axis2D.h"
#include "../enums/range_input_axes.h"

namespace dovahkit::subsystems::worldinput::util {
   class range_control_conflict_state {
      protected:
         static constexpr const uint8_t all_valid_axes_mask = []() -> uint8_t {
            uint8_t mask = 0;
            for (auto axis : { axis2D::x, axis2D::y }) {
               mask |= 1 << (int)axis;
            }
            return mask;
         }();

         uint8_t blocked_axes = 0;

      public:
         constexpr bool is_axis_blocked(axis2D) const;
         constexpr void set_axis_blocked(axis2D, bool);

         constexpr bool are_axes_blocked(range_input_axes) const; // returns `true` if all of the axes in question are blocked
         constexpr void set_axes_blocked(range_input_axes, bool);

         constexpr bool are_all_axes_blocked() const;
         constexpr void set_all_axes_blocked();

         constexpr bool are_any_axes_blocked() const { return blocked_axes != 0; }

         constexpr range_control_conflict_state& operator&=(const range_control_conflict_state&) noexcept;
         constexpr range_control_conflict_state operator&(const range_control_conflict_state&) const noexcept;

         constexpr range_control_conflict_state& operator|=(const range_control_conflict_state&) noexcept;
         constexpr range_control_conflict_state operator|(const range_control_conflict_state&) const noexcept;
         
         constexpr void constrain_value(QPointF&);
   };
}

#include "./range_control_conflict_state.inl"