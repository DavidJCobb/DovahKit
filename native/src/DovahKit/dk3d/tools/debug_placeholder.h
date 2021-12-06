#pragma once
#include <QString>
#include "_base.h"

namespace DK3D::tools {
   class debug_placeholder : public base { // intended for tools we haven't yet added to DovahKit; placeholder for pre-designed control schemes
      public:
         static constexpr const char* function_name = "debug_placeholder";
      protected:
         debug_placeholder() { this->setup(this); }

      public:
         static debug_placeholder& get() {
            static debug_placeholder instance;
            return instance;
         }
      public:
         struct options {
            QString text;
         };
      public:
         virtual void invoke(const InputResult&, const opaque_option_union&, DKVulkanCameraUpdate& camera_update) const override;
   };
}