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
         if (this->range.x != other.range.x)
            return false;
         if (this->range.y != other.range.y)
            return false;
      }
      if (this->constraints != other.constraints)
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
            magnitudes.x,
            magnitudes.y,
            magnitudes.z,
            range.x.axis,
            range.x.sign,
            range.y.axis,
            range.y.sign
         );
      }
      s.stream(
         constraints.frame,
         constraints.x,
         constraints.y,
         constraints.z
      );
   }
   constexpr void move_selection::options::stream(cobb::bitstreams::writer& s) const {
      s.stream(
         frame,
         follow_pointer
      );
      if (!follow_pointer) {
         s.stream(
            magnitudes.x,
            magnitudes.y,
            magnitudes.z,
            range.x.axis,
            range.x.sign,
            range.y.axis,
            range.y.sign
         );
      }
      s.stream(
         constraints.frame,
         constraints.x,
         constraints.y,
         constraints.z
      );
   }
   static_assert(cobb::bitstreams::round_trip_test<move_selection::options>, "Assert: round-trip bitstream serialization produces correct results.");

   constexpr void move_selection::response::by_temporality::merge(const by_temporality& from) {
      this->camera += from.camera;
      this->local  += from.local;
      this->world  += from.world;

      // Use the least-constrained movement.
      this->constraints.camera &= from.constraints.camera;
      this->constraints.local  &= from.constraints.local;
      this->constraints.world  &= from.constraints.world;

      auto& fp_src = from.follow_pointer;
      auto& fp_dst = this->follow_pointer;
      fp_dst.camera |= fp_src.camera;
      fp_dst.local  |= fp_src.local;
      fp_dst.world  |= fp_src.world;

      // Use the least-constrained movement.
      fp_dst.constraints.camera &= fp_src.constraints.camera;
      fp_dst.constraints.local  &= fp_src.constraints.local;
      fp_dst.constraints.world  &= fp_src.constraints.world;
   }
   //
   constexpr void move_selection::response::scale(double delta_seconds) {
      this->held.camera *= delta_seconds;
      this->held.local  *= delta_seconds;
      this->held.world  *= delta_seconds;
   }
   constexpr void move_selection::response::merge(const response& from) {
      this->held.merge(from.held);
      this->instant.merge(from.instant);
   }
}