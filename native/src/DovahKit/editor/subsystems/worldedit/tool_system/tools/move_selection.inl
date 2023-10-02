#pragma once
#include "./move_selection.h"

namespace dovahkit::subsystems::worldedit::tools {
   constexpr bool move_selection::options::operator==(const options& other) const noexcept {
      if (this->frame != other.frame)
         return false;
      if (this->follow_pointer != other.follow_pointer)
         return false;
      if (!this->follow_pointer) {
         if (this->magnitudes != other.magnitudes)
            return false;
         if (this->range != other.range)
            return false;
      }
      if (this->locked_axes != other.locked_axes)
         return false;
      return true;
   }

   constexpr void move_selection::options::stream(cobb::bitstreams::reader& s) {
      s.stream(
         frame,
         follow_pointer
      );
      if (!follow_pointer) {
         s.stream(
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
         follow_pointer
      );
      if (!follow_pointer) {
         s.stream(
            also_move_camera,
            magnitudes.x,
            magnitudes.y,
            magnitudes.z
         );
         
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
         &move_selection::options::follow_pointer,
         &move_selection::options::also_move_camera,
         &move_selection::options::magnitudes,
         &move_selection::options::locked_axes
      >(),
      "Assert: round-trip bitstream serialization produces correct results."
   );

   constexpr void move_selection::response::by_temporality::merge(const by_temporality& from) {
      this->magnitude += from.magnitude;

      this->camera.magnitude += from.camera.magnitude;

      auto& fp_src = from.follow_pointer;
      auto& fp_dst = this->follow_pointer;
      fp_dst.camera |= fp_src.camera;
      fp_dst.local  |= fp_src.local;
      fp_dst.world  |= fp_src.world;

      // Use the least-constrained movement.
      fp_dst.locked_axes.camera &= fp_src.locked_axes.camera;
      fp_dst.locked_axes.local  &= fp_src.locked_axes.local;
      fp_dst.locked_axes.world  &= fp_src.locked_axes.world;
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