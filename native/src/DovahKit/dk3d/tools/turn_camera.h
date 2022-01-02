#pragma once
#include "_base.h"
#include "../enums/camera_turn_axis.h"
#include "../enums/sign.h"

namespace DK3D::tools {
   class turn_camera : public base {
      public:
         static constexpr const char* function_name = "turn_camera";
         static constexpr compile_time_tool_options compile_time_options = {
            .use_strict_ordering = false,
         };
      protected:
         turn_camera() { this->setup(this); }

      public:
         static turn_camera& get() {
            static turn_camera instance;
            return instance;
         }
      public:
         struct options {
            struct {
               float yaw   = 0;
               float pitch = 0;
            } magnitudes;
            struct {
               camera_turn_axis input_x = camera_turn_axis::yaw;
               camera_turn_axis input_y = camera_turn_axis::pitch;
               sign x_sign = sign::positive;
               sign y_sign = sign::positive;
            } non_button;
         };
         struct results {
            float yaw   = 0.0; // per tick
            float pitch = 0.0; // per tick
            float roll  = 0.0; // per tick

            void scale(double delta_seconds);
            void merge(const results& from);
         };
      public:
         virtual void invoke(const input_result&, const opaque_option_union&, combined_tool_results&) const override;

         virtual bool has_options() const { return true; }
   };
}