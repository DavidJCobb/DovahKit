#pragma once
#include <glm/glm.hpp>
#include "./_base.h"

namespace dovah {
   class form_stub;
}

namespace dovahkit::subsystems::worldedit::tools {
   class debug_dump_landscape_details : public _base {
      public:
         static constexpr const char*          function_name = "debug_dump_landscape_details";
         static constexpr const cobb::eight_cc function_code = "DbgLands";

         static constexpr const compile_time_tool_options compile_time_options = {
            .use_strict_ordering = true,
         };
         static constexpr const bool is_raycast_sensitive = true;

      public:
         struct results {
            glm::vec3         hit_position = { 0, 0, 0 };
            dovah::form_stub* target       = nullptr;
         };

      public:
         static void invoke(const tool_invocation_cause&, const opaque_options_union&, tool_results_tuple&);
         static void invoke_for_hold_release(const opaque_options_union&, tool_results_tuple&);
   };
}