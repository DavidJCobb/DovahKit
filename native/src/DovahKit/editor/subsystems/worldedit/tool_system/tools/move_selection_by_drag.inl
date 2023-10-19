#pragma once
#include "./move_selection_by_drag.h"

namespace dovahkit::subsystems::worldedit::tools {
   constexpr void move_selection_by_drag::options::stream(cobb::bitstreams::reader& s) {
      s.stream(
         is_plane,
         frame,
         axis
      );
   }
   constexpr void move_selection_by_drag::options::stream(cobb::bitstreams::writer& s) const {
      s.stream(
         is_plane,
         frame,
         axis
      );
   }

   static_assert(cobb::bitstreams::round_trip_test<move_selection_by_drag::options>, "Assert: round-trip bitstream serialization produces correct results.");

   constexpr void move_selection_by_drag::response::scale(double delta_seconds) {
   }
   constexpr void move_selection_by_drag::response::merge(const response& from) {
      this->translate_by += from.translate_by;
   }
}