#pragma once
#include "_base.h"

namespace DovahKitDebug::features {
   struct pex_parsing_benchmarks : debug_feature {
      static constexpr const char* name = "PEX parsing benchmarks";
      static void execute(QWidget* from);
   };
}