#pragma once
#include "./_base.h"
#include "helpers/vector3.h"
#include "../../enums/axis3D.h"
#include "../../enums/reference_frame.h"
#include "../../enums/sign.h"
#include "editor/subsystems/worldinput/util/range_input_scales.h"

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
               reference_frame frame = reference_frame::camera;
               cobb::vector3<float> magnitudes;
               std::optional<worldinput::util::range_input_scales> range;

               constexpr void stream(cobb::bitstreams::reader&);
               constexpr void stream(cobb::bitstreams::writer&) const;
         };
         struct response {
            cobb::vector3<float> held;    // world-relative translation magnitude
            cobb::vector3<float> instant; // world-relative translation magnitude

            constexpr void scale(double delta_seconds);
            constexpr void merge(const response& from);
         };

      public:
         static void request(const tool_request_cause&, const options_union&, tool_response_tuple&);
         static void request_for_hold_release(const options_union&, tool_response_tuple&);

         static void invoke(const response&);
   };
}

#include "./move_camera.inl"