#pragma once
#include "helpers/bitstreams/reader.h"
#include "helpers/bitstreams/writer.h"
#include "editor/subsystems/worldedit/enums/sign.h"
#include "../tool_request_cause.h"

namespace dovahkit::subsystems::worldinput::util {
   struct range_input_scales_1D {
      public:
         constexpr bool operator==(const range_input_scales_1D& v) const noexcept = default;

         using sign = worldedit::sign;

      public:
         sign x = sign::positive;
         sign y = sign::positive;

         constexpr void scale(float &out, const tool_request_cause& input) const;

         constexpr void stream(cobb::bitstreams::reader&);
         constexpr void stream(cobb::bitstreams::writer&) const;
   };
}

#include "./range_input_scales_1D.inl"