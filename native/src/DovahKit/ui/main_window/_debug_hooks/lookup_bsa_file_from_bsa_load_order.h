#pragma once
#include "_base.h"

namespace DovahKitDebug::features {
   struct lookup_bsa_file_from_bsa_load_order : debug_feature {
      static constexpr const char* name = "Lookup single file from loaded BSAs";
      static void execute(QWidget* from);
   };
}