#pragma once
#include <glm/glm.hpp>
#include "./_base.h"
#include "../../enums/selection_operation.h"

namespace dovah {
   class form_stub;
}

namespace dovahkit::subsystems::worldedit::tools {
   class attempt_on_screen_selection : public _base {
      public:
         static constexpr const char*          function_name = "attempt_on_screen_selection";
         static constexpr const cobb::eight_cc function_code = "SelectAt";

         static constexpr const compile_time_tool_options compile_time_options = {
            .use_strict_ordering = true,
         };
         static constexpr const bool is_raycast_sensitive = true;

      public:
         struct options {
            protected:
               static constexpr const options_serialization_version serialization_version = 0;

            public:
               selection_operation operation = selection_operation::add;

               constexpr void read(options_serialization_version, cobb::streams::bitreader&);
               constexpr void write(cobb::streams::bitwriter&) const;
         };
         struct results {
            selection_operation operation;
            glm::vec3           hit_position = { 0, 0, 0 };
            dovah::form_stub*   target       = nullptr;
            //
            bool sweep = false; // for any non-button inputs; sweep the pointer over objects to modify selection state; Worldedit must track when an object is swept over/out
         };

      public:
         static void invoke(const tool_invocation_cause&, const opaque_options_union&, tool_results_tuple&);
         static void invoke_for_hold_release(const opaque_options_union&, tool_results_tuple&);
   };
}

#include "./attempt_on_screen_selection.inl"