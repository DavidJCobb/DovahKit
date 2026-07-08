#pragma once
#include "../base_file_load_warning.h"

#include "../_util.define.h"
namespace dovah::notices::file_load_warnings {
   class apparent_persistent_cell_not_flagged_as_persistent final : public base_file_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr apparent_persistent_cell_not_flagged_as_persistent(
            form_stub* world,
            form_stub& cell,
            bool auto_corrected
         ) :
            worldspace(world),
            persistent_cell(cell),
            problem_auto_corrected(auto_corrected)
         {}

         form_stub* worldspace;
         form_stub& persistent_cell;
         bool       problem_auto_corrected = false; // if `true`, we forced the flag to true on load
   };
}
#include "../_util.undef.h"