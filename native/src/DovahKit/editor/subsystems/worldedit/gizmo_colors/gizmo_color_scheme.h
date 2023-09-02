#pragma once
#include <array>
#include <string>
#include "helpers/color/rgb.h"

namespace dovahkit::subsystems::worldedit {
   struct gizmo_color_scheme {
      using rgb = cobb::color::rgb_bytes;

      std::string name; // utf-8
      union {
         std::array<rgb, 3> axes = {};
         struct {
            rgb axis_x;
            rgb axis_y;
            rgb axis_z;
         };
      };
      rgb highlight;

      constexpr bool operator==(const gizmo_color_scheme& o) const {
         if (this->highlight != o.highlight)
            return false;
         if (this->axes != o.axes)
            return false;
         return true;
      }
   };
}