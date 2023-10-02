#pragma once
#include "helpers/bitstreams/reader.h"
#include "helpers/bitstreams/writer.h"
#include "helpers/vector3.h"
#include "editor/subsystems/worldedit/enums/axis3D.h"
#include "editor/subsystems/worldedit/enums/sign.h"
#include "../tool_request_cause.h"

namespace dovahkit::subsystems::worldinput::util {
   struct range_input_scales {
      public:
         constexpr bool operator==(const range_input_scales& v) const noexcept = default;

         using axis3D = worldedit::axis3D;
         using sign   = worldedit::sign;

         struct input_axis {
            constexpr bool operator==(const input_axis& v) const noexcept = default;

            axis3D axis;
            sign   sign;

            constexpr void scale(cobb::vector3<float>& out, float input) const;
         };

      public:
         input_axis x = input_axis{ axis3D::x, sign::positive };
         input_axis y = input_axis{ axis3D::y, sign::positive };

         constexpr void scale(cobb::vector3<float>& out, const tool_request_cause& input) const;

         constexpr void stream(cobb::bitstreams::reader&);
         constexpr void stream(cobb::bitstreams::writer&) const;
   };
}

#include "./range_input_scales.inl"