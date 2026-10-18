#pragma once
#include "../_base.h"

namespace DovahKitDebug::features::mega_tests {
   struct mass_itm_cells_in_world : debug_feature {
      static constexpr const char* name = "Create ITMs of all cells in a given worldspace";
      static void execute(QWidget* from);
   };
}