#include "./condition.h"
#include "editor/subsystems/worldedit/core.h"

namespace dovahkit::subsystems::worldinput {
   /*static*/ control_scheme_condition control_scheme_condition::from_worldedit_state() {
      auto& worldedit = subsystems::worldedit::core::get();

      control_scheme_condition out;

      out.editor_modes = worldedit.get_editor_mode();

      {
         out.selection_count = selection_count_comparison_set{};
         out.selection_count.value().comparisons.push_back(selection_count_comparison_set::comparison_type{
            .op        = comparison_operator::less_or_equal,
            .comparand = worldedit.get_selected_refs().size()
         });
      }

      return out;
   }
}