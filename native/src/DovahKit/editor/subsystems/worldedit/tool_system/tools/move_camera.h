#pragma once
#include "./_base.h"
#include "helpers/macros/default_comparable_anonymous_struct.h"
#include "helpers/vector3.h"
#include "../../enums/axis3D.h"
#include "../../enums/reference_frame.h"
#include "../../enums/sign.h"

namespace dovahkit::subsystems::worldedit::tools {
   class move_camera : public _base {
      public:
         static constexpr const char*          function_name = "move_camera";
         static constexpr const cobb::eight_cc function_code = "MovCamra";
         static constexpr const compile_time_tool_options compile_time_options = {
            .use_strict_ordering = false,
         };

      public:
         struct options {
            public:
               constexpr bool operator==(const options& v) const noexcept = default;

            public:
               struct range_mapping {
                  constexpr bool operator==(const range_mapping& v) const noexcept = default;

                  axis3D axis;
                  sign   sign;
               };

            public:
               struct __anonymous_struct {
                  __anonymous_default_equality;
                  reference_frame baseline  = reference_frame::camera;
                  reference_frame selection = reference_frame::camera;
               } reference_frames;
               struct __anonymous_struct {
                  __anonymous_default_equality;
                  float x = 0;
                  float y = 0;
                  float z = 0;
               } magnitudes;
               struct __anonymous_struct {
                  __anonymous_default_equality;
                  range_mapping x = range_mapping{ axis3D::x, sign::positive };
                  range_mapping y = range_mapping{ axis3D::y, sign::negative };
               } range;

               constexpr void stream(cobb::bitstreams::reader&);
               constexpr void stream(cobb::bitstreams::writer&) const;
         };
         struct results {
            float x = 0;
            float y = 0;
            float z = 0;

            constexpr void scale(double delta_seconds);
            constexpr void merge(const results& from);
         };

      public:
         static void invoke(const tool_invocation_cause&, const opaque_options_union&, tool_results_tuple&);
         static void invoke_for_hold_release(const opaque_options_union&, tool_results_tuple&);
   };
}

#include "./move_camera.inl"
#include "helpers/macros/default_comparable_anonymous_struct.undef.h"