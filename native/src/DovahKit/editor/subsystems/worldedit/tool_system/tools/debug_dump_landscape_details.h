#pragma once
#include "../../enums/pointer_position_type.h"
#include "./_base.h"

namespace dovahkit::subsystems::worldedit::tools {
   class debug_dump_landscape_details : public _base {
      public:
         static constexpr const char* function_name = "debug_dump_landscape_details";
         static constexpr const compile_time_tool_options compile_time_options = {
            .use_strict_ordering = false,
         };

      public:
         struct options {
            pointer_position_type position  = pointer_position_type::mouse;
         };
         struct results {
            pointer_position_type position;
         };

      public:
         static void invoke(const tool_invocation_cause&, const opaque_options_union&, tool_results_tuple&);
         static void invoke_for_hold_release(const opaque_options_union&, tool_results_tuple&);
   };
}

#include "./move_camera.inl"