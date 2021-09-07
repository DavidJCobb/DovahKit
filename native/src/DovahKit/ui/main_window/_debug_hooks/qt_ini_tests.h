#pragma once
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

   extern void debug_qt_ini_helpers(QWidget* parent);
}
Q_DECLARE_METATYPE(DovahKitDebug::INIValueTestStruct);