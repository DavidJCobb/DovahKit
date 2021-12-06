#pragma once
#include "_base.h"
#include "../enums/CameraTurnAxis.h"
#include "../enums/ReferenceFrame.h"
#include "../enums/Sign.h"

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
               CameraTurnAxis input_x = CameraTurnAxis::Yaw;
               CameraTurnAxis input_y = CameraTurnAxis::Pitch;
               Sign x_sign = Sign::Positive;
               Sign y_sign = Sign::Positive;
            } non_button;
         };
      public:
         virtual void invoke(const InputResult&, const opaque_option_union&, DKVulkanCameraUpdate& camera_update) const override;

         virtual bool has_options() const { return true; }
   };
}