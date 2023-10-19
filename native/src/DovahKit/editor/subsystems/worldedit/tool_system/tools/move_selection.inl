#pragma once
#include "./move_selection.h"

namespace dovahkit::subsystems::worldedit::tools {
   constexpr bool move_selection::options::operator==(const options& other) const noexcept {
      if (this->frame != other.frame)
         return false;
      if (this->also_move_camera != other.also_move_camera)
         return false;
      if (this->magnitudes != other.magnitudes)
         return false;
      if (this->range != other.range)
         return false;
      if (this->locked_axes != other.locked_axes)
         return false;
      return true;
   }

   constexpr void move_selection::options::stream(cobb::bitstreams::reader& s) {
      s.stream(
         frame,
         also_move_camera,
         magnitudes.x,
         magnitudes.y,
         magnitudes.z
      );
      {  // range
         bool presence = false;
         s.stream(presence);
         if (presence) {
            this->range.emplace();
            s.stream(this->range.value());
         }
      }
      s.stream(
         locked_axes.frame,
         locked_axes.x,
         locked_axes.y,
         locked_axes.z
      );
   }
   constexpr void move_selection::options::stream(cobb::bitstreams::writer& s) const {
      s.stream(
         frame,
         also_move_camera,
         magnitudes.x,
         magnitudes.y,
         magnitudes.z
      );
      {  // range
         s.stream(range.has_value());
         if (range.has_value())
            s.stream(range.value());
      }
      s.stream(
         locked_axes.frame,
         locked_axes.x,
         locked_axes.y,
         locked_axes.z
      );
   }

   static_assert(
      cobb::bitstreams::round_trip_test_with_targeted_scramble<
         move_selection::options,
         //
         // MSVC cannot perform a bitcast on a std::optional, which is understandable, frankly. 
         // We need to use the "targeted scramble" test and explicitly specify which members 
         // are safe to scramble as part of the test procedure:
         //
         &move_selection::options::frame,
         &move_selection::options::also_move_camera,
         &move_selection::options::magnitudes,
         &move_selection::options::locked_axes
      >(),
      "Assert: round-trip bitstream serialization produces correct results."
   );

   constexpr void move_selection::response::by_temporality::merge(const by_temporality& from) {
      this->magnitude += from.magnitude;
      this->camera.magnitude += from.camera.magnitude;
   }
   //
   constexpr void move_selection::response::scale(double delta_seconds) {
      this->held.magnitude *= delta_seconds;
      this->held.camera.magnitude *= delta_seconds;
   }
   constexpr void move_selection::response::merge(const response& from) {
      this->held.merge(from.held);
      this->instant.merge(from.instant);
   }
}