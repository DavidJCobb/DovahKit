#pragma once
#include <string>
#include "./_base.h"

namespace dovahkit::subsystems::worldedit::tools {
   class debug_print : public _base {
      public:
         static constexpr const char*          function_name = "debug_print";
         static constexpr const cobb::eight_cc function_code = "DbgPrint";
         static constexpr const compile_time_tool_options compile_time_options = {
            .use_strict_ordering = false,
         };

      public:
         struct options {
            protected:
               static constexpr const options_serialization_version serialization_version = 0;

            public:
               std::string text;

               constexpr void read(options_serialization_version, cobb::streams::bitreader&);
               constexpr void write(cobb::streams::bitwriter&) const;
         };
         struct results {
            std::string text;

            constexpr void merge(const results& from) {
               this->text += '\n';
               this->text += from.text;
            }
         };

      public:
         static void invoke(const tool_invocation_cause&, const opaque_options_union&, tool_results_tuple&);
         static void invoke_for_hold_release(const opaque_options_union&, tool_results_tuple&);
   };
}

#include "./debug_print.inl"