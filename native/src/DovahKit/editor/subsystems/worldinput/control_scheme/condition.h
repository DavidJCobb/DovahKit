#pragma once
#include <optional>
#include "helpers/enum_flags.h"
#include "editor/subsystems/worldedit/enums/editor_mode.h"
#include "../enums/comparison_operator.h"
#include "../util/numeric_comparisons.h"

namespace cobb::bitstreams {
   class reader;
   class writer;
}

namespace dovahkit::subsystems::worldinput {
   class control_scheme_condition {
      public:
         using editor_mode = ::dovahkit::subsystems::worldedit::editor_mode;
         using value_type = ::dovahkit::subsystems::worldedit::editor_mode;

         using selection_count_comparison_set = util::comparison_set<size_t>;

         std::optional<cobb::enum_flags<editor_mode, 3>> editor_modes;
         std::optional<selection_count_comparison_set>   selection_count;

         static control_scheme_condition from_worldedit_state();

         constexpr bool impossible() const noexcept;

         constexpr bool operator==(const control_scheme_condition&) const noexcept;

         constexpr control_scheme_condition& operator&=(const control_scheme_condition&);

         constexpr control_scheme_condition operator&(const control_scheme_condition& o) {
            return control_scheme_condition(*this) &= o;
         }

         constexpr void stream(cobb::bitstreams::reader&);
         constexpr void stream(cobb::bitstreams::writer&) const;
   };
}

#include "./condition.inl"