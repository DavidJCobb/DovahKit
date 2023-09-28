#pragma once
#include "helpers/vector3.h"
#include "./_base.h"
#include "../../enums/camera_orbit_target.h"
#include "../../enums/camera_turn_axis.h"
#include "../../enums/sign.h"

namespace dovahkit::subsystems::worldedit::tools {
   class orbit_camera : public _base {
      public:
         static constexpr const char*          function_name = "orbit_camera";
         static constexpr const cobb::eight_cc function_code = "ObtCamra";
         static constexpr const compile_time_tool_options compile_time_options = {
            .use_strict_ordering = true,
         };

      public:
         struct options {
            public:
               constexpr bool operator==(const options& v) const noexcept;

               struct range_mapping {
                  constexpr bool operator==(const range_mapping& v) const noexcept = default;

                  camera_turn_axis axis;
                  sign             sign;
               };

            public:
               struct _ {
                  constexpr bool operator==(const _& v) const noexcept = default;
                  float yaw   = 0;
                  float pitch = 0;
               } magnitudes;
               struct {
                  range_mapping x = range_mapping{ camera_turn_axis::pitch, sign::positive };
                  range_mapping y = range_mapping{ camera_turn_axis::yaw,   sign::negative };
               } range;
               camera_orbit_target target = camera_orbit_target::primary_selection;

               constexpr void stream(cobb::bitstreams::reader&);
               constexpr void stream(cobb::bitstreams::writer&) const;
         };
         struct response {
            camera_orbit_target target = camera_orbit_target::primary_selection;
            //
            cobb::vector3<float> held;    // x, y, z = pitch, roll, yaw
            cobb::vector3<float> instant; // x, y, z = pitch, roll, yaw

            constexpr void scale(double delta_seconds);
            constexpr void merge(const response& from);
         };

      public:
         static void request(const tool_request_cause&, const opaque_options_union&, tool_response_tuple&);
         static void request_for_hold_release(const opaque_options_union&, tool_response_tuple&);
   };
}

#include "./orbit_camera.inl"