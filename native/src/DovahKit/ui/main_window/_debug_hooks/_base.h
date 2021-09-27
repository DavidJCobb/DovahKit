#pragma once
#include <QWidget>

namespace DovahKitDebug {
   struct debug_feature {
      static constexpr const char* name = "uninitialized";
      static void execute(QWidget* from) {}
   };
}