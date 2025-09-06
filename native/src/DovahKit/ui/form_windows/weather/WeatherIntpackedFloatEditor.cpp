#include "./WeatherIntpackedFloatEditor.h"
#include <array>

void WeatherIntpackedFloatEditor::abandonWidgets() {
   if (this->hasWidgets()) {
      QObject::disconnect(this->slider,  nullptr, this, nullptr);
      QObject::disconnect(this->spinbox, nullptr, this, nullptr);
      this->slider  = nullptr;
      this->spinbox = nullptr;
   }
}
bool WeatherIntpackedFloatEditor::hasWidgets() const {
   //
   // Can only set the widgets in unison, so only need to check one.
   //
   return this->slider != nullptr;
}
void WeatherIntpackedFloatEditor::setWidgets(QSlider& slider, QDoubleSpinBox& spinbox) {
   this->abandonWidgets();

   this->slider  = &slider;
   this->spinbox = &spinbox;

   slider.setRange(0, 0xFF);
   spinbox.setRange(this->_minimum, this->_maximum);
   QObject::connect(&slider, &QSlider::valueChanged, &spinbox, [this](int v) {
      const auto blocker = QSignalBlocker(this->spinbox);
      this->spinbox->setValue(this->byteToFloat(v));
   });
   QObject::connect(&spinbox, qOverload<double>(&QDoubleSpinBox::valueChanged), &slider, [this](double v) {
      this->slider->setValue(this->floatToByte(v));
   });
}

void WeatherIntpackedFloatEditor::bindTo(uint8_t& target) {
   if (!this->hasWidgets())
      return;

   QObject::connect(this->slider, &QSlider::valueChanged, this->spinbox, [&target](int v) {
      target = v;
   });
   const auto blockers = std::array{
      QSignalBlocker(slider),
      QSignalBlocker(spinbox),
   };
   this->slider->setValue(target);
   this->spinbox->setValue(this->byteToFloat(target));
}

void WeatherIntpackedFloatEditor::setMinimum(float v) {
   if (this->_minimum == v)
      return;
   this->_minimum = v;
   if (v <= this->_maximum)
      this->_on_range_changed();
}
void WeatherIntpackedFloatEditor::setMaximum(float v) {
   if (this->_maximum == v)
      return;
   this->_maximum = v;
   if (v >= this->_minimum)
      this->_on_range_changed();
}
void WeatherIntpackedFloatEditor::setRange(float a, float b) {
   if (a == this->_minimum && b == this->_maximum)
      return;
   this->_minimum = a;
   this->_maximum = b;
   this->_on_range_changed();
}

float WeatherIntpackedFloatEditor::byteToFloat(uint8_t v) const {
   if (this->slider->invertedAppearance()) {
      v = 255 - v;
   }
   float out = (float)v / 255.0F;
   out *= (this->_maximum - this->_minimum);
   out += this->_minimum;
   return out;
}
uint8_t WeatherIntpackedFloatEditor::floatToByte(float v) const {
   if (this->_maximum == this->_minimum)
      return 0;
   v -= this->_minimum;
   v /= (this->_maximum - this->_minimum);
   v *= 255;
   if (this->slider->invertedAppearance()) {
      v = 255 - v;
   }
   return (uint8_t)v;
}

void WeatherIntpackedFloatEditor::_on_range_changed() {
   if (!this->hasWidgets())
      return;

   const auto blockers = std::array{
      QSignalBlocker(this->slider),
      QSignalBlocker(this->spinbox),
   };
   this->spinbox->setRange(this->_minimum, this->_maximum);
   this->spinbox->setValue(this->byteToFloat(this->slider->value()));
}