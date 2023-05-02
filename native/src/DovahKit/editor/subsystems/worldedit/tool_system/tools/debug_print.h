#pragma once
#include <string>
#include "./_base.h"

namespace dovahkit::subsystems::worldedit::tools {
   class debug_print : public _base {
      public:
         static constexpr const char* function_name = "debug_print";
         static constexpr const compile_time_tool_options compile_time_options = {
            .use_strict_ordering = false,
         };

      public:
         struct options {
            std::string text;
         };
         struct results {
            std::string text;

            constexpr void merge(const results& from) {
               this->text = from.text + this->text;
            }
         };

      public:
         static void invoke(const tool_invocation_cause&, const opaque_options_union&, tool_results_tuple&);
         static void invoke_for_hold_release(const opaque_options_union&, tool_results_tuple&);
   };
}

#include "./move_camera.inl"