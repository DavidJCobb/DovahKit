#pragma once
#include "./_base.h"
#include "../enums/axis3D.h"
#include "../enums/reference_frame.h"
#include "../enums/sign.h"

namespace dovahkit::subsystems::worldinput::tools {
   class move_camera : public base {
      public:
         static constexpr const char* function_name = "move_camera";
         static constexpr compile_time_tool_options compile_time_options = {
            .use_strict_ordering = false,
         };
      protected:
         move_camera() { this->setup(this); }

      public:
         static move_camera& get() {
            static move_camera instance;
            return instance;
         }
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
               axis3D input_x = axis3D::x;
               axis3D input_y = axis3D::y;
               sign   x_sign  = sign::positive;
               sign   y_sign  = sign::negative;
            } non_button;
         };
         struct results {
            float x = 0;
            float y = 0;
            float z = 0;

            void scale(double delta_seconds);
            void merge(const results& from);
         };
      public:
         virtual void invoke(const input_result&, const opaque_option_union&, combined_tool_results&) const override;

         virtual bool has_options() const { return true; }
   };
}