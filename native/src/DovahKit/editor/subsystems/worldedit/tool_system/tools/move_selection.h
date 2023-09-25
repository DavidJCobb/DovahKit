#pragma once
#include "./_base.h"
#include "helpers/vector3.h"
#include "../../enums/axis3D.h"
#include "../../enums/reference_frame.h"
#include "../../enums/sign.h"

namespace dovahkit::subsystems::worldedit::tools {
   class move_selection : public _base {
      public:
         static constexpr const char*          function_name = "move_selection";
         static constexpr const cobb::eight_cc function_code = "MovSlctd";
         static constexpr const compile_time_tool_options compile_time_options = {
            .use_strict_ordering = false,
         };

      public:
         struct options {
            public:
               struct constraint_data {
                  constexpr bool operator==(const constraint_data& v) const noexcept = default;

                  reference_frame frame = reference_frame::current; // "current" = "use same frame as movement"
                  bool x = false; // If `true`, movement will not occur along the X-axis.
                  bool y = false; // If `true`, movement will not occur along the Y-axis.
                  bool z = false; // If `true`, movement will not occur along the Z-axis.
               };

               struct range_mapping {
                  constexpr bool operator==(const range_mapping& v) const noexcept = default;

                  axis3D axis;
                  sign   sign;
               };

            public:
               reference_frame frame = reference_frame::world;
               //
               bool follow_pointer = false; // mutually exclusive with `magnitudes` and `range`
               //
               cobb::vector3<float> magnitudes;
               struct {
                  range_mapping x = range_mapping{ axis3D::x, sign::positive };
                  range_mapping y = range_mapping{ axis3D::y, sign::negative };
               } range;
               //
               constraint_data constraints;

               constexpr bool operator==(const options& v) const noexcept;

               constexpr void stream(cobb::bitstreams::reader&);
               constexpr void stream(cobb::bitstreams::writer&) const;
         };
         struct response {
            struct by_temporality {
               cobb::vector3<float> camera;
               cobb::vector3<float> local;
               cobb::vector3<float> world;
               struct {
                  uint8_t camera = 0;
                  uint8_t local  = 0;
                  uint8_t world  = 0;
               } constraints;
               //
               struct {
                  bool camera = false;
                  bool local  = false;
                  bool world  = false;
                  struct {
                     uint8_t camera = 0;
                     uint8_t local  = 0;
                     uint8_t world  = 0;
                  } constraints;
               } follow_pointer;

               constexpr void merge(const by_temporality& from);
            };

            by_temporality held;
            by_temporality instant;

            constexpr void scale(double delta_seconds);
            constexpr void merge(const response& from);
         };

      public:
         static void request(const tool_request_cause&, const opaque_options_union&, tool_response_tuple&);
         static void request_for_hold_release(const opaque_options_union&, tool_response_tuple&);

         static void invoke(const response&);
   };
}

#include "./move_selection.inl"