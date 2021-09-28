#pragma once
#include "_base.h"

namespace DovahKitDebug::features {
   struct get_record_size_stats : debug_feature {
      static constexpr const char* name = "Get record size stats";
      static void execute(QWidget* from);
   };
}