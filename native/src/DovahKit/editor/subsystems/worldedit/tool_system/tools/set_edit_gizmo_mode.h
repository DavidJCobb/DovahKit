#pragma once
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
            protected:
               static constexpr const options_serialization_version serialization_version = 0;

            public:
               struct {
                  reference_frame a = reference_frame::current; // current == no change
                  reference_frame b = reference_frame::current;
               } frame;
               struct {
                  gizmo_mode a = gizmo_mode::none;
                  gizmo_mode b = gizmo_mode::none;
               } gizmo;
               bool toggle_frame = false; // If true and we're already on the "a" frame, we switch to the "b" frame.
               bool toggle_gizmo = false; // If true and we're already on the "a" mode,  we switch to the "b" mode.
               bool modify_gizmo = false; // Since there's no "current" value for the gizmo_mode enum.

               constexpr void read(options_serialization_version, cobb::streams::bitreader&);
               constexpr void write(cobb::streams::bitwriter&) const;
         };
         struct results : public options {
         };

      public:
         static void invoke(const tool_invocation_cause&, const opaque_options_union&, tool_results_tuple&);
         static void invoke_for_hold_release(const opaque_options_union&, tool_results_tuple&);
   };
}

#include "./set_edit_gizmo_mode.inl"