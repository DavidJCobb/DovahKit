#pragma once
#include <optional>
#include "helpers/enum_flags.h"
#include "editor/subsystems/worldedit/enums/editor_mode.h"
#include "editor/subsystems/worldedit/enums/gizmo_mode.h"
#include "./enums/comparison_operator.h"
#include "./util/numeric_comparisons.h"

namespace cobb::bitstreams {
   class reader;
   class writer;
}

namespace dovahkit::subsystems::worldinput {
   class condition_set {
      public:
         using editor_mode = ::dovahkit::subsystems::worldedit::editor_mode;
         using gizmo_mode  = ::dovahkit::subsystems::worldedit::gizmo_mode;

         using selection_count_comparison_set = util::comparison_set<size_t>;

         std::optional<cobb::enum_flags<editor_mode, 4>> editor_modes;
         std::optional<cobb::enum_flags<gizmo_mode, 4>>  gizmo_modes;
         std::optional<selection_count_comparison_set>   selection_count;

         static condition_set from_worldedit_state();

         constexpr bool impossible() const noexcept;

         constexpr bool operator==(const condition_set&) const noexcept;

         constexpr condition_set& operator&=(const condition_set&);

         constexpr condition_set operator&(const condition_set& o) {
            return condition_set(*this) &= o;
         }

         constexpr void stream(cobb::bitstreams::reader&);
         constexpr void stream(cobb::bitstreams::writer&) const;
   };
}

#include "./condition_set.inl"