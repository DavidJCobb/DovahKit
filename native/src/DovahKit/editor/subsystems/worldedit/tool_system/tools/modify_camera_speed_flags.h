#pragma once
#include "./_base.h"
#include "../../enums/bool_operation.h"

namespace dovahkit::subsystems::worldedit::tools {
   class modify_camera_speed_flags : public _base {
      public:
         static constexpr const char* function_name = "modify_camera_speed_flags";
         static constexpr const compile_time_tool_options compile_time_options = {
            .use_strict_ordering = false,
         };

      public:
         struct options {
            bool_operation boost     = bool_operation::no_op;
            bool_operation precision = bool_operation::no_op;
         };
         struct results {
            bool_operation boost     = bool_operation::no_op;
            bool_operation precision = bool_operation::no_op;

            void merge(const results& from);
         };
      public:
         static void invoke(const tool_invocation_cause&, const opaque_options_union&, tool_results_tuple&);
         static void invoke_for_hold_release(const opaque_options_union&, tool_results_tuple&);
   };
}