#pragma once
#include <QDoubleSpinBox>
#include <QSpinBox>

namespace cobb::qt {
   inline void remove_spinbox_bounds(QDoubleSpinBox* widget) {
      widget->setRange(-FLT_MAX, FLT_MAX);
   }
   inline void remove_spinbox_bounds(QSpinBox* widget) {
      widget->setRange(std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max());
   }
}