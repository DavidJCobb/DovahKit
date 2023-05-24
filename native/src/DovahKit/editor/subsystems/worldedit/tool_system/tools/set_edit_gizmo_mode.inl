#pragma once
#include "./set_edit_gizmo_mode.h"

namespace dovahkit::subsystems::worldedit::tools {
   constexpr void set_edit_gizmo_mode::options::stream(cobb::bitstreams::reader& s) {
      s.stream(frame.a);
      s.stream(toggle_frame);
      if (this->toggle_frame) {
         s.stream(frame.b);
      } else {
         frame.b = reference_frame::current;
      }

      s.stream(modify_gizmo);
      if (this->modify_gizmo) {
         s.stream(gizmo.a);
         s.stream(toggle_gizmo);
         if (this->toggle_gizmo) {
            s.stream(gizmo.b);
         } else {
            gizmo.b = gizmo_mode::none;
         }
      } else {
         this->gizmo = {};
         this->toggle_gizmo = false;
      }
   }
   constexpr void set_edit_gizmo_mode::options::stream(cobb::bitstreams::writer& s) const {
      s.stream(frame.a);
      s.stream(toggle_frame);
      if (this->toggle_frame) {
         s.stream(frame.b);
      }

      s.stream(modify_gizmo);
      if (this->modify_gizmo) {
         s.stream(gizmo.a);
         s.stream(toggle_gizmo);
         if (this->toggle_gizmo)
            s.stream(gizmo.b);
      }
   }
   static_assert(cobb::bitstreams::round_trip_test<set_edit_gizmo_mode::options>, "Assert: round-trip bitstream serialization produces correct results.");
}

