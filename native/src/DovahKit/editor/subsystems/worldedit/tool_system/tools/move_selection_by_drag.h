#pragma once
#include <variant>
#include "./_base.h"
//
#include "helpers/vector3.h"
#include "../../enums/reference_frame.h"
#include "editor/subsystems/worldinput/util/range_input_scales.h"

namespace dovahkit::subsystems::worldedit::tools {
   class move_selection_by_drag : public _base {
      public:
         static constexpr const char*          function_name = "move_selection_by_drag";
         static constexpr const cobb::eight_cc function_code = "DrgMvSel";
         static constexpr const compile_time_tool_options compile_time_options = {
            .use_strict_ordering = false,
         };

      public:
         struct options {
            public:
               struct drag_axis {
                  constexpr bool operator==(const drag_axis& v) const noexcept = default;

                  reference_frame frame;
                  axis3D axis;
               };
               struct drag_plane {
                  constexpr bool operator==(const drag_plane& v) const noexcept = default;

                  reference_frame frame;
                  axis3D axis_u;
                  axis3D axis_v;
               };

            public:
               reference_frame frame = reference_frame::world;
               axis3D axis = axis3D::z;

               // Indicates whether we're dragging along an axis or a plane.
               // If true, then `axis` is the drag plane's normal (i.e. axis == z means the XY-plane).
               // If false, then `axis` is... well, the drag axis!
               bool is_plane = false;

               constexpr bool operator==(const options& v) const noexcept = default;

               constexpr void stream(cobb::bitstreams::reader&);
               constexpr void stream(cobb::bitstreams::writer&) const;
         };
         struct response {
            cobb::vector3<float> translate_by;

            constexpr void scale(double delta_seconds);
            constexpr void merge(const response& from);
         };

      public:
         static void request(const tool_request_cause&, const opaque_options_union&, tool_response_tuple&);
         static void request_for_hold_release(const opaque_options_union&, tool_response_tuple&);

         static void invoke(const response&);
   };
}

#include "./move_selection_by_drag.inl"