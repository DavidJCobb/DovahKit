#pragma once
#include "helpers/macros/default_comparable_anonymous_struct.h"
#include "./_base.h"
#include "../../enums/camera_turn_axis.h"
#include "../../enums/reference_frame.h"
#include "../../enums/sign.h"

namespace dovahkit::subsystems::worldedit::tools {
   class turn_camera : public _base {
      public:
         static constexpr const char*          function_name = "turn_camera";
         static constexpr const cobb::eight_cc function_code = "TrnCamra";
         static constexpr const compile_time_tool_options compile_time_options = {
            .use_strict_ordering = false,
         };

      public:
         struct options {
            public:
               constexpr bool operator==(const options& v) const noexcept = default;

            public:
               struct __anonymous_struct {
                  __anonymous_default_equality;
                  float yaw   = 0;
                  float pitch = 0;
               } magnitudes;
               struct __anonymous_struct {
                  __anonymous_default_equality;
                  struct __anonymous_struct {
                     __anonymous_default_equality;
                     camera_turn_axis axis = camera_turn_axis::yaw;
                     sign sign = sign::positive;
                  } x;
                  struct __anonymous_struct {
                     __anonymous_default_equality;
                     camera_turn_axis axis = camera_turn_axis::pitch;
                     sign sign = sign::positive;
                  } y;
               } range;

               constexpr void stream(cobb::bitstreams::reader&);
               constexpr void stream(cobb::bitstreams::writer&) const;
         };
         struct response {
            float yaw   = 0.0; // per tick
            float pitch = 0.0; // per tick
            float roll  = 0.0; // per tick

            constexpr void scale(double delta_seconds);
            constexpr void merge(const response& from);
         };

      public:
         static void request(const tool_request_cause&, const opaque_options_union&, tool_response_tuple&);
         static void request_for_hold_release(const opaque_options_union&, tool_response_tuple&);
   };
}

#include "./turn_camera.inl"
#include "helpers/macros/default_comparable_anonymous_struct.undef.h"