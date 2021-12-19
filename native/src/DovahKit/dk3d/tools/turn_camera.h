#pragma once
#include "_base.h"
#include "../enums/camera_turn_axis.h"
#include "../enums/sign.h"

namespace DK3D::tools {
   class turn_camera : public base {
      public:
         static constexpr const char* function_name = "turn_camera";
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
      public:
         virtual void invoke(const InputResult&, const opaque_option_union&, DKVulkanCameraUpdate& camera_update) const override;

         virtual bool has_options() const { return true; }
   };
}