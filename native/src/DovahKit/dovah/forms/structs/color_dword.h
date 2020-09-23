#pragma once
#include <cstdint>
#include "../_common.h"

namespace dovah::loaded_forms {
   union color_t {
      struct {
         uint8_t r;
         uint8_t g;
         uint8_t b;
         uint8_t unused;
      };
      uint32_t hex = 0;
      //
      bool load(tes_subrecord_reader&);
      void save(tes_subrecord_writer&);
   };
}