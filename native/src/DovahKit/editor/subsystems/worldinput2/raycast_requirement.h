#pragma once
#include "editor/subsystems/worldedit/enums/axis3D.h"
#include "vulkan/enums/gizmo_mode.h"
#include "./enums/optional_yn.h"

namespace dovahkit::subsystems::worldedit {
   struct raycast_result;
}
namespace dovahkit::subsystems::worldinput2 {
   using raycast_result = worldedit::raycast_result;
}

namespace dovahkit::subsystems::worldinput2 {
   struct raycast_requirement {
      public:
         using axis3D     = worldedit::axis3D;
         using gizmo_mode = vulkanDK::gizmo_mode;

         enum class timing_type {
            on_control_down,
            per_frame,
         };

      public:
         timing_type timing = timing_type::on_control_down;
         bool        fail_if_target_changes = false; // If set, then the target is checked every frame; if the target changes, the associated bind is blocked.
         struct {
            axis3D     edit_gizmo_axis   : 2 = axis3D::x;
            gizmo_mode edit_gizmo_mode   : 2 = gizmo_mode::none;
            bool       landscapes        : 1 = false;
            bool       nothing           : 1 = false; // If set, then a raycast must be performed, and "nothing" is one of the things it's allowed to hit for the requirements to be met.
            bool       object_references : 1 = false;
         } targets;
         struct {
            optional_yn selected : 2 = optional_yn::unspecified;
         } target_options;

         // Returns true if there exist no raycast requirements.
         constexpr bool empty() const noexcept {
            return (
               targets.edit_gizmo_mode != gizmo_mode::none
            || targets.landscapes
            || targets.nothing
            || targets.object_references
            ) == false;
         }

         bool is_satisfied_by(const raycast_result&) const;
   };
}