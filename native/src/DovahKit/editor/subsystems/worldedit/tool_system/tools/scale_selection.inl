#pragma once
#include "./scale_selection.h"

namespace dovahkit::subsystems::worldedit::tools {
   constexpr void scale_selection::options::stream(cobb::bitstreams::reader& s) {
      s.stream(
         mod,
         scale_all_together
      );
      {  // range
         bool presence = false;
         s.stream(presence);
         if (presence) {
            this->range.emplace();
            s.stream(range.value());
         }
      }
   }
   constexpr void scale_selection::options::stream(cobb::bitstreams::writer& s) const {
      s.stream(
         mod,
         scale_all_together
      );
      {  // range
         s.stream(range.has_value());
         if (range.has_value()) {
            s.stream(range.value());
         }
      }
   }

   static_assert(
      cobb::bitstreams::round_trip_test_with_targeted_scramble<
         scale_selection::options,
         //
         // MSVC cannot perform a bitcast on a std::optional, which is understandable, frankly. 
         // We need to use the "targeted scramble" test and explicitly specify which members 
         // are safe to scramble as part of the test procedure:
         //
         &scale_selection::options::mod,
         &scale_selection::options::scale_all_together
      >(),
      "Assert: round-trip bitstream serialization produces correct results."
   );

   constexpr void scale_selection::response::by_temporality::merge(const by_temporality& from) {
      this->unified    += from.unified;
      this->individual += from.individual;
   }

   constexpr void scale_selection::response::scale(double delta_seconds) {
      this->held.unified    *= delta_seconds;
      this->held.individual *= delta_seconds;
   }
   constexpr void scale_selection::response::merge(const response& from) {
      this->held.unified    += from.held.unified;
      this->held.individual += from.held.individual;

      this->instant.unified    += from.instant.unified;
      this->instant.individual += from.instant.individual;
   }
}