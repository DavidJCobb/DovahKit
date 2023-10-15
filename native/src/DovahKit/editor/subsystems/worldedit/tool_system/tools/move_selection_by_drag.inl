#pragma once
#include "./move_selection_by_drag.h"

namespace dovahkit::subsystems::worldedit::tools {
   constexpr void move_selection_by_drag::options::stream(cobb::bitstreams::reader& s) {
      bool is_plane;
      s.stream(is_plane);

      if (!is_plane) {
         auto& dfn = this->drag_along.emplace<drag_axis>();
         s.stream(
            dfn.frame,
            dfn.axis
         );
      } else {
         auto& dfn = this->drag_along.emplace<drag_plane>();
         s.stream(
            dfn.frame,
            dfn.axis_u,
            dfn.axis_v
         );
      }
   }
   constexpr void move_selection_by_drag::options::stream(cobb::bitstreams::writer& s) const {
      bool is_plane = std::holds_alternative<drag_plane>(this->drag_along);
      s.stream(is_plane);

      if (!is_plane) {
         assert(std::holds_alternative<drag_axis>(this->drag_along));
         auto& dfn = std::get<drag_axis>(this->drag_along);
         s.stream(
            dfn.frame,
            dfn.axis
         );
      } else {
         auto& dfn = std::get<drag_plane>(this->drag_along);
         s.stream(
            dfn.frame,
            dfn.axis_u,
            dfn.axis_v
         );
      }
   }

   // TODO: devise a round-trip test that would properly let us handle all cases of a std::variant
   //       maybe something that just takes an already-constructed value as an argument and uses 
   //       that as a control group, with no scrambling on read

   constexpr void move_selection_by_drag::response::scale(double delta_seconds) {
   }
   constexpr void move_selection_by_drag::response::merge(const response& from) {
      this->translate_by += from.translate_by;
   }
}