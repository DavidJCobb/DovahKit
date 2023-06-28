#pragma once
#include "editor/subsystems/worldedit/enums/editor_mode.h"

namespace cobb::bitstreams {
   class reader;
   class writer;
}

namespace dovahkit::subsystems::worldinput2 {
   class control_scheme_condition {
      public:
         using value_type = ::dovahkit::subsystems::worldedit::editor_mode;

         constexpr control_scheme_condition() {}
         constexpr control_scheme_condition(value_type em) : mode(em) {}

         value_type mode = value_type::objects;

         constexpr bool operator==(const control_scheme_condition&) const noexcept;

         constexpr void stream(cobb::bitstreams::reader&);
         constexpr void stream(cobb::bitstreams::writer&) const;
   };
}

#include "./condition.inl"