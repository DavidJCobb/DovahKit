#pragma once
#include <array>
#include "../_common.h"

namespace dovah::loaded_forms {
   union color_floats {
      struct {
         float r;
         float g;
         float b;
         float a;
      };
      std::array<float, 4> values = { 0, 0, 0, 0 };

      constexpr color_floats& operator=(const color_floats&) noexcept = default;
      constexpr color_floats& operator=(const std::array<float, 4>& v) noexcept {
         this->values = v;
      }
      constexpr bool operator==(const color_floats&) const noexcept = default;
      constexpr bool operator==(const std::array<float, 4>& v) const noexcept { return this->values == v; }
      
      bool load(tes_subrecord_reader&);
      void save(tes_subrecord_writer&);
   };
}