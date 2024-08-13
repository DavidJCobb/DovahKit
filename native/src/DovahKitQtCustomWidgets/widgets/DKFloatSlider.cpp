#include "./DKFloatSlider.h"
#include <QBoxLayout>

DKFloatSlider::DKFloatSlider(QWidget* parent) : QWidget(parent) {
   this->_slider = new QSlider(this);
   {
      auto* layout = new QHBoxLayout(this);
      layout->setContentsMargins(0, 0, 0, 0);
      layout->setSizeConstraint(QLayout::SizeConstraint::SetMinAndMaxSize);
      layout->addWidget(this->_slider);
   }
   QObject::connect(this->_slider, QOverload<int>::of(&QSlider::valueChanged), this, [this]() {
      emit valueChanged(this->value());
   });
   QObject::connect(this->_slider, &QSlider::sliderMoved, this, [this]() {
      emit sliderMoved(this->sliderPosition());
   });

   this->setFocusPolicy(Qt::FocusPolicy::TabFocus);
   this->setFocusProxy(this->_slider);
}
DKFloatSlider::DKFloatSlider(Qt::Orientation o, QWidget* parent) : DKFloatSlider(parent) {
   this->setOrientation(o);
}

/*virtual*/ QSize DKFloatSlider::sizeHint() const /*override*/ {
   return this->_slider->sizeHint();
}
/*virtual*/ QSize DKFloatSlider::minimumSizeHint() const /*override*/ {
   return this->_slider->minimumSizeHint();
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
void DKFloatSlider::setRange(float minimum, float maximum) {
   if (minimum == this->minimum() && maximum == this->maximum())
      return;
   this->_properties.range.min = minimum;
   this->_properties.range.max = maximum;
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
   return (float)this->_slider->value() / _float_to_int_mult();
}
void DKFloatSlider::setValue(float v) {
   this->_slider->setValue(v * _float_to_int_mult());
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

bool DKFloatSlider::isSliderDown() const {
   return this->_slider->isSliderDown();
}
void DKFloatSlider::setSliderDown(bool v) {
   this->_slider->setSliderDown(v);
}

float DKFloatSlider::sliderPosition() const {
   return (float)this->_slider->sliderPosition() / _float_to_int_mult();
}
void DKFloatSlider::setSliderPosition(float v) {
   this->_slider->setSliderPosition(v * _float_to_int_mult());
}

int DKFloatSlider::_float_to_int_mult() const {
   int mult = 1;
   for (size_t i = 0; i < this->_properties.decimals; ++i)
      mult *= 10;
   return mult;
}
void DKFloatSlider::_recalculate_slider_units() {
   auto mult = _float_to_int_mult();

   this->_slider->setSingleStep(mult);
   this->_slider->setRange(mult * this->minimum(), mult * this->maximum());

   int i_tick = mult * this->tickInterval();
   this->_slider->setTickInterval(i_tick);
   if (i_tick) {
      this->_slider->setPageStep(i_tick);
   } else {
      this->_slider->setPageStep(mult * 10);
   }
}