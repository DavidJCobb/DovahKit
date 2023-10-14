#pragma once
#include "helpers/vector3.h"
#include "./_base.h"
#include "../../enums/camera_turn_axis.h"
#include "../../enums/reference_frame.h"
#include "../../enums/sign.h"
#include "editor/subsystems/worldinput/util/range_input_scales.h"

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
               cobb::vector3<float> magnitudes; // pitch, roll, yaw
               std::optional<worldinput::util::range_input_scales> range;

               constexpr void stream(cobb::bitstreams::reader&);
               constexpr void stream(cobb::bitstreams::writer&) const;
         };
         struct response {
            cobb::vector3<float> held;    // x, y, z = pitch, roll, yaw
            cobb::vector3<float> instant; // x, y, z = pitch, roll, yaw

            constexpr void scale(double delta_seconds);
            constexpr void merge(const response& from);
         };

      public:
         static void request(const tool_request_cause&, const opaque_options_union&, tool_response_tuple&);
         static void request_for_hold_release(const opaque_options_union&, tool_response_tuple&);

         static void invoke(const response&);
   };
}

#include "./turn_camera.inl"