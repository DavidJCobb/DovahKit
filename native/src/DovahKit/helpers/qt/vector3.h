#pragma once
#include <QDoubleSpinBox>
#include "../vector3.h"

namespace cobb::qt {
   extern void bring_vector3_to_ui(const cobb::vector3<float>&, QDoubleSpinBox* x, QDoubleSpinBox* y, QDoubleSpinBox* z);
   extern void get_vector3_from_ui(cobb::vector3<float>&, const QDoubleSpinBox* x, const QDoubleSpinBox* y, const QDoubleSpinBox* z);
}
