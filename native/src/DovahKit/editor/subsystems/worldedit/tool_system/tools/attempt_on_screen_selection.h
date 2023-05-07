#pragma once
#include <glm/glm.hpp>
#include "./_base.h"
#include "../../enums/pointer_position_type.h"
#include "../../enums/selection_operation.h"

namespace dovah {
   class form_stub;
}

namespace dovahkit::subsystems::worldedit::tools {
   class attempt_on_screen_selection : public _base {
      public:
         static constexpr const char* function_name = "attempt_on_screen_selection";

         static constexpr const compile_time_tool_options compile_time_options = {
            .use_strict_ordering = true,
         };
         static constexpr const bool is_raycast_sensitive = true;

      public:
         struct options {
            selection_operation   operation = selection_operation::add;
            pointer_position_type position  = pointer_position_type::mouse;
         };
         struct results {
            selection_operation   operation;
            pointer_position_type position;
            glm::vec3 hit_position = { 0, 0, 0 };
            dovah::form_stub* target = nullptr;
            //
            bool sweep = false; // for any non-button inputs; sweep the pointer over objects to modify selection state; Worldedit must track when an object is swept over/out
         };

      public:
         static void invoke(const tool_invocation_cause&, const opaque_options_union&, tool_results_tuple&);
         static void invoke_for_hold_release(const opaque_options_union&, tool_results_tuple&);
   };
}