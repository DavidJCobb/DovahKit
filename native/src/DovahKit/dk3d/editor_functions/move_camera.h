#pragma once
#include "_base.h"
#include "../enums/Axis3D.h"
#include "../enums/ReferenceFrame.h"
#include "../enums/Sign.h"

namespace DK3D::editor_functions {
   class move_camera : public base {
      public:
         static move_camera& get() {
            static move_camera instance;
            return instance;
         }
      public:
         struct options {
            bool also_translate_selection = false;
            struct {
               ReferenceFrame baseline  = ReferenceFrame::World;
               ReferenceFrame selection = ReferenceFrame::World;
            } reference_frames;
            struct {
               float x = 0;
               float y = 0;
               float z = 0;
            } magnitudes;
            struct {
               Axis3D input_x = Axis3D::X;
               Axis3D input_y = Axis3D::Y;
               Sign   x_sign  = Sign::Positive;
               Sign   y_sign  = Sign::Negative;
            } non_button;
         };
      public:
         virtual void invoke(const InputResult&, const opaque_option_union&, DKVulkanCameraUpdate& camera_update) override;
   };
}