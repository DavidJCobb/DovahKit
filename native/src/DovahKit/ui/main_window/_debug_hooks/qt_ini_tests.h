#pragma once
#include "_base.h"

namespace DovahKitDebug::features {
   struct qt_ini_tests : debug_feature {
      static constexpr const char* name = "Test Qt INI helpers";
      static void execute(QWidget* from);
   };
}

#include <QString>
#include <QWidget>

namespace DovahKitDebug {
   struct INIValueTestStruct {
      int     number = 0;
      QString text   = "";
      //
      static INIValueTestStruct fromString(const QString&);
      QString toString() const noexcept;
   };
}
Q_DECLARE_METATYPE(DovahKitDebug::INIValueTestStruct);