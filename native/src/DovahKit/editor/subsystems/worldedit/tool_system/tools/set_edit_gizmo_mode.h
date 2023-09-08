#pragma once
#include "helpers/macros/default_comparable_anonymous_struct.h"
#include "./_base.h"
#include "../../enums/gizmo_mode.h"
#include "../../enums/reference_frame.h"

namespace dovahkit::subsystems::worldedit::tools {
   class set_edit_gizmo_mode : public _base {
      public:
         static constexpr const char*          function_name = "set_edit_gizmo_mode";
         static constexpr const cobb::eight_cc function_code = "EdtGizmo";
         static constexpr const compile_time_tool_options compile_time_options = {
            .use_strict_ordering = true,
         };

      public:
         struct options {
            public:
               constexpr bool operator==(const options& v) const noexcept = default;

            public:
               struct __anonymous_struct {
                  __anonymous_default_equality;
                  reference_frame a = reference_frame::current; // current == no change
                  reference_frame b = reference_frame::current;
               } frame;
               struct __anonymous_struct {
                  __anonymous_default_equality;
                  gizmo_mode a = gizmo_mode::none;
                  gizmo_mode b = gizmo_mode::none;
               } gizmo;
               bool toggle_frame = false; // If true and we're already on the "a" frame, we switch to the "b" frame.
               bool toggle_gizmo = false; // If true and we're already on the "a" mode,  we switch to the "b" mode.
               bool modify_gizmo = false; // Since there's no "current" value for the gizmo_mode enum.

               constexpr void stream(cobb::bitstreams::reader&);
               constexpr void stream(cobb::bitstreams::writer&) const;
         };
         struct response : public options {
         };

      public:
         static void request(const tool_request_cause&, const opaque_options_union&, tool_response_tuple&);
         static void request_for_hold_release(const opaque_options_union&, tool_response_tuple&);
   };
}

#include "./set_edit_gizmo_mode.inl"
#include "helpers/macros/default_comparable_anonymous_struct.undef.h"