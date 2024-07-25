#include "./DKFloatSlider.h"
#include <QBoxLayout>

DKFloatSlider::DKFloatSlider(QWidget* parent) : QWidget(parent) {
   this->_slider = new QSlider(this);
   {
      auto* layout = new QHBoxLayout(this);
      layout->addWidget(this->_slider);
   }
   QObject::connect(this->_slider, QOverload<int>::of(&QSlider::valueChanged), this, [this]() {
      emit valueChanged(this->value());
   });

   this->setFocusPolicy(Qt::FocusPolicy::TabFocus);
   this->setFocusProxy(this->_slider);
}
DKFloatSlider::DKFloatSlider(Qt::Orientation o, QWidget* parent) : DKFloatSlider(parent) {
   this->setOrientation(o);
}

bool DKFloatSlider::hasTracking() const {
   return this->_slider->hasTracking();
}
void DKFloatSlider::setTracking(bool v) {
   return this->_slider->setTracking(v);
}

void DKFloatSlider::setDecimals(unsigned int v) {
   if (v == this->decimals())
      return;
   this->_properties.decimals = v;
   this->_recalculate_slider_units();
}
void DKFloatSlider::setMaximum(float v) {
   if (v == this->maximum())
      return;
   this->_properties.range.max = v;
   this->_recalculate_slider_units();
}
void DKFloatSlider::setMinimum(float v) {
   if (v == this->minimum())
      return;
   this->_properties.range.min = v;
   this->_recalculate_slider_units();
}

void DKFloatSlider::setTickInterval(float v) {
   if (v == this->tickInterval())
      return;
   this->_properties.tick_interval = v;
   this->_recalculate_slider_units();
}

QSlider::TickPosition DKFloatSlider::tickPosition() {
   return this->_slider->tickPosition();
}
void DKFloatSlider::setTickPosition(QSlider::TickPosition v) {
   this->_slider->setTickPosition(v);
}

float DKFloatSlider::value() const {
   auto f_v = (float)this->_slider->value();
   for (size_t i = 0; i < this->_properties.decimals; ++i) {
      f_v /= 10;
   }
   return f_v;
}
void DKFloatSlider::setValue(float v) {
   for (size_t i = 0; i < this->_properties.decimals; ++i)
      v *= 10;
   this->_slider->setValue((int)v);
}

bool DKFloatSlider::invertedAppearance() const {
   return this->_slider->invertedAppearance();
}
bool DKFloatSlider::invertedControls() const {
   return this->_slider->invertedControls();
}
Qt::Orientation DKFloatSlider::orientation() const {
   return this->_slider->orientation();
}
//
void DKFloatSlider::setInvertedAppearance(bool v) {
   this->_slider->setInvertedAppearance(v);
}
void DKFloatSlider::setInvertedControls(bool v) {
   this->_slider->setInvertedControls(v);
}
void DKFloatSlider::setOrientation(Qt::Orientation v) {
   this->_slider->setOrientation(v);
}

bool DKFloatSlider::isSliderDown() {
   return this->_slider->isSliderDown();
}

void DKFloatSlider::_recalculate_slider_units() {
   float span = this->_properties.range.max - this->_properties.range.min;

   int i_step_single = 1;
   int i_tick;
   int i_min;
   int i_max;
   {
      float f_tick = this->tickInterval();
      float f_min  = this->minimum();
      float f_max  = this->maximum();
      for (size_t i = 0; i < this->_properties.decimals; ++i) {
         i_step_single *= 10;
         f_tick *= 10;
         f_min  *= 10;
         f_max  *= 10;
      }
      i_tick = f_tick;
      i_min  = f_min;
      i_max  = f_max;
   }
   this->_slider->setSingleStep(i_step_single);
   this->_slider->setRange(i_min, i_max);
   this->_slider->setTickInterval(i_tick);
}