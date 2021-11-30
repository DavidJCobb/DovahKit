#pragma once
#include "_base.h"

namespace DK3D::editor_functions {
   class debug_log : public base {
      public:
         static debug_log& get() {
            static debug_log instance;
            return instance;
         }
      public:
         struct options {
            int number = 0;
         };
      public:
         virtual void invoke(const InputResult&, const opaque_option_union&, DKVulkanCameraUpdate& camera_update) override;
   };
}