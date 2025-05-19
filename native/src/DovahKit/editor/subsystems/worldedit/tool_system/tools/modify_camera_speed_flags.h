#pragma once
#include "./_base.h"
#include "../../enums/bool_operation.h"

namespace dovahkit::subsystems::worldedit::tools {
   class modify_camera_speed_flags : public _base {
      public:
         static constexpr const char*          function_name = "modify_camera_speed_flags";
         static constexpr const cobb::eight_cc function_code = "CamSpeed";
         static constexpr const compile_time_tool_options compile_time_options = {
            .use_strict_ordering = false,
         };

      public:
         struct options {
            public:
               constexpr bool operator==(const options& v) const noexcept = default;

            public:
               bool_operation boost     = bool_operation::no_op;
               bool_operation precision = bool_operation::no_op;

               constexpr void stream(cobb::bitstreams::reader&);
               constexpr void stream(cobb::bitstreams::writer&) const;
         };
         struct response {
            bool_operation boost     = bool_operation::no_op;
            bool_operation precision = bool_operation::no_op;

            void merge(const response& from);
         };
      public:
         static void request(const tool_request_cause&, const options_union&, tool_response_tuple&);
         static void request_for_hold_release(const options_union&, tool_response_tuple&);

         static void invoke(const response&);
   };
}

#include "./modify_camera_speed_flags.inl"