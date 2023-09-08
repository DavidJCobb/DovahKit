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
            public:
               constexpr bool operator==(const options& v) const noexcept = default;

            public:
               static constexpr const size_t max_length = 1023;

               std::string text;

               constexpr void stream(cobb::bitstreams::reader&);
               constexpr void stream(cobb::bitstreams::writer&) const;
         };
         struct response {
            std::string text;

            constexpr void merge(const response& from) {
               this->text += '\n';
               this->text += from.text;
            }
         };

      public:
         static void request(const tool_request_cause&, const opaque_options_union&, tool_response_tuple&);
         static void request_for_hold_release(const opaque_options_union&, tool_response_tuple&);

         static void invoke(const response&);
   };
}

#include "./debug_print.inl"