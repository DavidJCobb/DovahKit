#include "DelayedAvailabilityButton.h"
#include <QPainter>

DelayedAvailabilityButton::DelayedAvailabilityButton(QWidget* parent) : QPushButton(parent) {
   auto palette = this->palette();
   palette.setCurrentColorGroup(QPalette::Disabled);
   this->_spinnerConfig.background = Qt::transparent;
   this->_spinnerConfig.border     = palette.dark().color();
   this->_spinnerConfig.fill       = palette.color(QPalette::Highlight);
   //
   this->_paintTimer.setInterval(33);
   this->_paintTimer.start();
   //
   this->_timer.setSingleShot(true);
   this->_timer.setInterval(3000);
   if (this->isEnabled()) {
      this->setDisabled(true);
      this->_timer.start();
   }
   QObject::connect(&this->_timer, &QTimer::timeout, this, [this]() {
      this->setEnabled(true);
   });
   QObject::connect(&this->_paintTimer, &QTimer::timeout, this, [this]() {
      if (this->_timer.isActive())
         this->update();
   });
}

int DelayedAvailabilityButton::delay() const noexcept {
   return this->_timer.interval();
}
void DelayedAvailabilityButton::setDelay(int ms) {
   this->_timer.setInterval(ms);
}
void DelayedAvailabilityButton::restartDelay() {
   QPushButton::setDisabled(true);
   this->_timer.start();
   this->_paintTimer.start();
}

void DelayedAvailabilityButton::setDisabled(bool state) {
   this->setEnabled(!state);
}
void DelayedAvailabilityButton::setEnabled(bool state) {
   QPushButton::setEnabled(state);
   this->_timer.stop();
}

void DelayedAvailabilityButton::paintEvent(QPaintEvent* event) {
   QPushButton::paintEvent(event);
   if (this->isEnabled() || !this->_timer.isActive())
      return;
   QPainter painter(this);
   //
   float progress = 1.0F - ((float)(this->_timer.remainingTime() - _completionTimeBuffer) / (float)this->_timer.interval());
   if (progress > 1.0F)
      progress = 1.0F;
   //
   if (this->_style == Style::Bar) {
      auto color = QColor(80, 176, 32);
      auto shine = QColor(96, 224, 48);
      //
      int   to     = this->width() * progress;
      int   height = this->height();
      //
      QPen pen;
      pen.setBrush(QBrush(color, Qt::SolidPattern));
      pen.setWidth(2);
      pen.setCosmetic(true);
      height -= pen.width() / 2;
      painter.setPen(pen);
      painter.drawLine(0, height, to, height);
      //
      QRect bounds = this->rect();
      bounds.setWidth(to);
      bounds.setTop(bounds.y() + height);
      QLinearGradient gradient(bounds.topLeft(), bounds.bottomRight());
      gradient.setColorAt(0, Qt::transparent);
      gradient.setColorAt(1, shine);
      pen.setBrush(QBrush(gradient));
      painter.setPen(pen);
      painter.drawLine(0, height, to, height); // draw the line in reverse, so the "shine" is nearest to the "front"
   } else if (this->_style == Style::Spinner) {
      QPen pen;
      pen.setColor(this->_spinnerConfig.background);
      pen.setWidthF(3.0F);
      pen.setCapStyle(Qt::RoundCap);
      pen.setCosmetic(true);
      //
      float cross_size = this->height();
      if (cross_size > this->width())
         cross_size = this->width();
      QRectF spinner;
      float margin = cross_size * 0.15F;
      if (cross_size < 64)
         margin = 3.0F;
      margin += pen.widthF();
      float diameter = cross_size - (margin * 2);
      spinner.setTop(margin);
      spinner.setLeft(margin);
      spinner.setHeight(diameter);
      spinner.setWidth(diameter);
      //
      painter.setRenderHint(QPainter::Antialiasing, true);
      {
         QPen border_pen = pen;
         border_pen.setWidthF(1.0F);
         border_pen.setColor(this->_spinnerConfig.border);
         //
         auto   thick = pen.widthF() / 2.0F + 0.5F;
         QRectF rect  = spinner;
         rect += QMargins(thick, thick, thick, thick);
         painter.setPen(border_pen);
         painter.drawArc(rect, 0 * 16, 360 * 16);
         rect = spinner;
         rect -= QMargins(thick, thick, thick, thick);
         painter.setPen(border_pen);
         painter.drawArc(rect, 0 * 16, 360 * 16);
      }
      //
      painter.setPen(pen);
      painter.drawArc(spinner, 0 * 16, 360 * 16);
      //
      pen.setColor(this->_spinnerConfig.fill);
      if (this->_spinnerConfig.fillCoversBorder)
         pen.setWidthF(pen.width() + 2.0F);
      painter.setPen(pen);
      painter.drawArc(spinner, 90 * 16, -progress * (360 * 16)); // zero degrees is at 3 o'clock. angles are counterclockwise and can be negated for clockwise.
   }
}