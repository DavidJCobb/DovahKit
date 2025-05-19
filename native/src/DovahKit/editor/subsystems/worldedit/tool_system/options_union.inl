#pragma once
#include "./options_union.h"

namespace dovahkit::subsystems::worldedit::tools {
   constexpr void options_union::stream(cobb::bitstreams::reader& s) {
      bool presence;
      s.stream(presence);
      if (!presence)
         return;
      std::visit(
         [&s](auto& v) {
            s.stream(v);
         },
         *this
      );
   }
   constexpr void options_union::stream(cobb::bitstreams::writer& s) const {
      bool wrote = false;
      std::visit(
         [&s, &wrote](auto& v) {
            s.stream(true); // presence bool
            s.stream(v);
            wrote = true;
         },
         *this
      );
      if (!wrote) {
         s.stream(false); // presence bool
      }
   }
}