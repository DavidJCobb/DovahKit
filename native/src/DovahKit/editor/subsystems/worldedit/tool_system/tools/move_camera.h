#pragma once
#include "./_base.h"
#include "../../enums/axis3D.h"
#include "../../enums/reference_frame.h"
#include "../../enums/sign.h"

namespace dovahkit::subsystems::worldedit::tools {
   class move_camera : public _base {
      public:
         static constexpr const char* function_name = "move_camera";
         static constexpr const compile_time_tool_options compile_time_options = {
            .use_strict_ordering = false,
         };

      public:
         struct options {
            bool also_translate_selection = false;
            struct {
               reference_frame baseline  = reference_frame::camera;
               reference_frame selection = reference_frame::camera;
            } reference_frames;
            struct {
               float x = 0;
               float y = 0;
               float z = 0;
            } magnitudes;
            struct {
               struct {
                  axis3D axis = axis3D::x;
                  sign   sign = sign::positive;
               } x;
               struct {
                  axis3D axis = axis3D::y;
                  sign   sign = sign::negative;
               } y;
            } range;
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