#pragma once
#include "_base.h"

namespace DovahKitDebug::features {
   struct list_none_stubs : debug_feature {
      static constexpr const char* name = "List all dangling form references";
      static void execute(QWidget* from);
   };
}