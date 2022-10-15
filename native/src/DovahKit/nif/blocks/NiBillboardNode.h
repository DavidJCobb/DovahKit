#pragma once
#include <type_traits>
#include "NiNode.h"

namespace nifDK::block_types {
   class NiBillboardNode : public NiNode {
      public:
         static constexpr const char* const type_name = "NiBillboardNode";
      public:
         enum class billboard_mode : uint16_t {
            face_camera,        // Match billboard to camera-forward. Minimized rotation.
            rotate_about_up,    // Rotate only about the up-axis.
            rigid_face_camera,  // Match billboard to camera-forward. Non-minimized rotation.
            always_face_center,
         };

         billboard_mode mode = billboard_mode::face_camera;

         virtual void parse(file_reader&) override;
   };
}