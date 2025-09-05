#include "./detection_loudness.h"
#include <QCoreApplication>
#include "dovah/data/detection_loudness.h"

namespace ui::enum_dropdown_configs {
   extern void detection_loudness(QComboBox* widget) {
      widget->clear();
      widget->addItem(QCoreApplication::translate("dovah::detection_loudness", "Silent"),    (int)dovah::detection_loudness::silent);
      widget->addItem(QCoreApplication::translate("dovah::detection_loudness", "Normal"),    (int)dovah::detection_loudness::normal);
      widget->addItem(QCoreApplication::translate("dovah::detection_loudness", "Loud"),      (int)dovah::detection_loudness::loud);
      widget->addItem(QCoreApplication::translate("dovah::detection_loudness", "Very Loud"), (int)dovah::detection_loudness::very_loud);
   }
}