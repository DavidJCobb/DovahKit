#include "vector3.h"

namespace cobb::qt {
   void bring_vector3_to_ui(const cobb::vector3<float>& vec, QDoubleSpinBox* x, QDoubleSpinBox* y, QDoubleSpinBox* z) {
      const auto blocker_x = QSignalBlocker(x);
      const auto blocker_y = QSignalBlocker(y);
      const auto blocker_z = QSignalBlocker(z);
      x->setValue(vec.x);
      y->setValue(vec.y);
      z->setValue(vec.z);
   }
   void get_vector3_from_ui(cobb::vector3<float>& vec, const QDoubleSpinBox* x, const QDoubleSpinBox* y, const QDoubleSpinBox* z) {
      vec.x = x->value();
      vec.y = y->value();
      vec.z = z->value();
   }
}