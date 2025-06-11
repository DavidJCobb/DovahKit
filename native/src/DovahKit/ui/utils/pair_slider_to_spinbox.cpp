#include "./pair_slider_to_spinbox.h"

namespace ui {
   extern void pair_slider_to_spinbox(QSlider* slider, QSpinBox* spinbox) {
      slider->setRange(spinbox->minimum(), spinbox->maximum());
      QObject::connect(slider, &QSlider::valueChanged, spinbox, [spinbox](int v) {
         spinbox->setValue(v);
      });
      QObject::connect(spinbox, qOverload<int>(&QSpinBox::valueChanged), slider, [slider](int v) {
         const auto blocker = QSignalBlocker(slider);
         slider->setValue(v);
      });
   }
   extern void pair_slider_to_spinbox(DKFloatSlider* slider, QDoubleSpinBox* spinbox) {
      slider->setRange(spinbox->minimum(), spinbox->maximum());
      QObject::connect(slider, &DKFloatSlider::valueChanged, spinbox, [spinbox](float v) {
         spinbox->setValue(v);
      });
      QObject::connect(spinbox, qOverload<double>(&QDoubleSpinBox::valueChanged), slider, [slider](double v) {
         const auto blocker = QSignalBlocker(slider);
         slider->setValue(v);
      });
   }
}