#pragma once
#include "./_base.h"
//
#include <optional>
#include "helpers/vector3.h"
#include "../../enums/axis3D.h"
#include "../../enums/reference_frame.h"
#include "../../enums/sign.h"
#include "editor/subsystems/worldinput/util/range_input_scales_1D.h"

namespace dovahkit::subsystems::worldedit::tools {
   class scale_selection : public _base {
      public:
         static constexpr const char*          function_name = "scale_selection";
         static constexpr const cobb::eight_cc function_code = "ScaleSel";
         static constexpr const compile_time_tool_options compile_time_options = {
            .use_strict_ordering = false,
         };

      public:
         struct options {
            public:
               float mod = 0; // should be +/-; it's added to the current scale, not multiplied in
               bool  scale_all_together = false; // if `true`, we also scale the distances between the refs
               std::optional<worldinput::util::range_input_scales_1D> range;

               constexpr bool operator==(const options& v) const noexcept = default;

               constexpr void stream(cobb::bitstreams::reader&);
               constexpr void stream(cobb::bitstreams::writer&) const;
         };
         struct response {
            struct by_temporality {
               float individual = 0;
               float unified    = 0;

               constexpr void merge(const by_temporality&);
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

#include "./scale_selection.inl"