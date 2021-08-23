#include "DKDoubleSlider.h"
#include <QPaintEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionSlider>

DKDoubleSlider::DKDoubleSlider(QWidget* parent) : QWidget(parent) {
}

void DKDoubleSlider::paintEvent(QPaintEvent* event) {
   auto  options = QStyleOptionSlider();
   auto  painter = QPainter(this);
   auto* style   = this->style();
   style->drawComplexControl(QStyle::ComplexControl::CC_Slider, &options, &painter, this);
}
void DKDoubleSlider::_setUpStyleOptions(QStyleOptionSlider& options) const noexcept {
   options.initFrom(this);
   //
   // TODO: QStyleOptionSlider is built around everything being ints. Can we make our 
   // own QStyleOptionComplex subclass?
   //
   double gap = this->state.maximum - this->state.minimum;
   options.maximum = this->state.maximum;
   options.minimum = this->state.minimum;
   options.orientation  = this->state.orientation;
   options.pageStep     = this->state.pageStep;
   options.singleStep   = this->state.singleStep;
   options.sliderValue  = this->state.value;
   options.tickInterval = this->state.tickInterval;
   options.tickPosition = this->state.tickPosition;
}