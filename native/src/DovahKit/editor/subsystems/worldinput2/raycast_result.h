#pragma once
#include <glm/glm.hpp>
#include <optional>
#include "editor/subsystems/worldedit/enums/axis3D.h"
#include "vulkan/enums/gizmo_mode.h"
#include "./inputs/button.h"
#include "./chrono.h"

namespace dovah {
   class form_stub;
}

namespace dovahkit::subsystems::worldinput2 {
   struct raycast_result {
      public:
         using axis3D     = worldedit::axis3D;
         using gizmo_mode = vulkanDK::gizmo_mode;

      public:
         std::optional<glm::vec3> hit_position;
         struct {
            struct {
               axis3D     axis = axis3D::x;
               gizmo_mode mode = gizmo_mode::none;
            } edit_gizmo;
            dovah::form_stub* form = nullptr;
         } target_info;
   };

   struct raycast_result_per_key : public raycast_result {
      inputs::button button;
      timestamp_t    when;
   };
}