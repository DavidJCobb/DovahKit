#pragma once
#include <string>
#include "./_base.h"

namespace dovah {
   class form_stub;
}

namespace dovahkit::subsystems::worldedit::tools {
   class debug_dump_raycast : public _base {
      public:
         static constexpr const char*          function_name = "debug_dump_raycast";
         static constexpr const cobb::eight_cc function_code = "DbgRayca";

         static constexpr const compile_time_tool_options compile_time_options = {
            .use_strict_ordering = true,
         };
         static constexpr const bool is_raycast_sensitive = true;

      public:
         // no options or results

      public:
         static void invoke(const tool_invocation_cause&, const opaque_options_union&, tool_results_tuple&);
         static void invoke_for_hold_release(const opaque_options_union&, tool_results_tuple&);
   };
}