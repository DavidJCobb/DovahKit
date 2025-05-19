#include "./DKDelayedEnableButton.h"
#include <QEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>
#include <QStylePainter>

DKDelayedEnableButton::DKDelayedEnableButton(QWidget* parent) : QPushButton(parent) {
   auto palette = this->palette();
   palette.setCurrentColorGroup(QPalette::Disabled);

   this->_spinner_options.background = Qt::transparent;
   this->_spinner_options.border     = palette.dark().color();
   this->_spinner_options.fill       = palette.color(QPalette::Highlight);
   
   this->_timers.repaint.setInterval(33);
   this->_timers.repaint.start();
   //
   this->_timers.enable.setSingleShot(true);
   this->_timers.enable.setInterval(3000);
   if (this->isEnabled()) {
      this->setDisabled(true);
      this->_timers.enable.start();
   }
   #if !defined(QT_PLUGIN)
      QObject::connect(&this->_timers.enable, &QTimer::timeout, this, [this]() {
         this->setEnabled(true);
      });
   #endif
   QObject::connect(&this->_timers.repaint, &QTimer::timeout, this, [this]() {
      if (this->_timers.enable.isActive()) {
         this->update();
      }
   });
}

float DKDelayedEnableButton::progress() const noexcept {
   #if defined(QT_PLUGIN)
      if (this->isEnabled())
         return 1.0F;
      return 0.5F;
   #else
      if (!this->_timers.enable.isActive())
         return 1.0F;

      float progress = 1.0F - ((float)(this->_timers.enable.remainingTime() - _completionTimeBuffer) / (float)this->_timers.enable.interval());
      if (progress > 1.0F)
         progress = 1.0F;
      return progress;
   #endif
}
unsigned int DKDelayedEnableButton::secondsRemaining() const noexcept {
   #if defined(QT_PLUGIN)
      if (this->isEnabled())
         return 0;

      return this->delay() / 1000 / 2;
   #else
      if (!this->_timers.enable.isActive())
         return 0;

      auto ms = this->_timers.enable.remainingTime();
      auto s  = ms / 1000;
      if (ms % 1000)
         ++s;

      return s;
   #endif
}

int DKDelayedEnableButton::delay() const noexcept {
   return this->_timers.enable.interval();
}
void DKDelayedEnableButton::setDelay(int ms) {
   this->_timers.enable.setInterval(ms);
}
void DKDelayedEnableButton::restartDelay() {
   QPushButton::setDisabled(true);
   this->_timers.enable.start();
   this->_timers.repaint.start();
}
void DKDelayedEnableButton::queueEnable() {
   if (this->isEnabled())
      return;
   this->_timers.enable.start();
   this->_timers.repaint.start();
}

void DKDelayedEnableButton::setCounterStyle(CounterStyle after) {
   auto prior = this->_style;
   if (prior == after)
      return;
   this->_style = after;
   this->updateGeometry();
   this->update();
}

void DKDelayedEnableButton::changeEvent(QEvent* event) {
   QWidget::changeEvent(event);
   if (event->type() == QEvent::Type::EnabledChange) {
      if (!this->isEnabled()) {
         this->_timers.enable.stop();
      }
   }
}
void DKDelayedEnableButton::paintEvent(QPaintEvent* event) {
   bool is_counting_down = 
      #if defined(QT_PLUGIN)
         !this->isEnabled()
      #else
         (!this->isEnabled() && this->_timers.enable.isActive())
      #endif
   ;

   {
      QStylePainter      style_painter(this);
      QStyleOptionButton option;

      this->initStyleOption(&option);
      if (this->_style == CounterStyle::Number && is_counting_down) {
         option.text = this->_effective_label();
      }

      style_painter.drawControl(QStyle::CE_PushButton, option);
   }

   if (!is_counting_down || this->_style == CounterStyle::Number)
      return;

   QPainter painter(this);
   
   float progress = this->progress();
   
   if (this->_style == CounterStyle::Bar) {
      this->_draw_bar(painter, progress);
   } else if (this->_style == CounterStyle::Spinner) {
      float cross_size = this->height();
      if (cross_size > this->width())
         cross_size = this->width();

      float margin = cross_size * 0.15F;
      if (cross_size < 64)
         margin = 3.0F;
      margin += this->_spinner_options.thickness;

      float diameter = cross_size - (margin * 2);

      QRectF spinner;
      spinner.setTop(margin);
      spinner.setLeft(margin);
      spinner.setHeight(diameter);
      spinner.setWidth(diameter);

      this->_draw_spinner(painter, progress, spinner);
   }
}

QSize DKDelayedEnableButton::sizeHint() const {
   auto metrics = this->fontMetrics();
   auto result  = QPushButton::sizeHint();
   if (this->_style == CounterStyle::Number) {
      result.rwidth() += metrics.width(tr("%1 (%2)").arg("").arg(10));
   } else if (this->_style == CounterStyle::Spinner) {
      float diameter;
      {
         float cross_size = this->height();
         if (cross_size > this->width())
            cross_size = this->width();

         float margin = cross_size * 0.15F;
         if (cross_size < 64)
            margin = 3.0F;
         margin += this->_spinner_options.thickness;

         diameter = cross_size - (margin * 2);
      }

      int padding_w = 0;
      {
         auto style = this->style();

         QStyleOptionButton option;
         this->initStyleOption(&option);

         padding_w = style->pixelMetric(QStyle::PM_ButtonMargin, &option, this) * 2;
      }

      auto displayed_width = metrics.width(this->text());
      auto needed_padding  = (diameter + metrics.width(" ")) * 2; // double, because text is centered but the spinner is side-aligned
      if (result.width() - displayed_width - padding_w < needed_padding) {
         result.rwidth() += needed_padding;
      }
   }
   return result;
}

void DKDelayedEnableButton::_draw_bar(QPainter& painter, float progress) {
   auto color = QColor(80, 176, 32);
   auto shine = QColor(96, 224, 48);
   
   int to     = this->width() * progress;
   int height = this->height();
   
   QPen pen;
   pen.setBrush(QBrush(color, Qt::SolidPattern));
   pen.setWidth(2);
   pen.setCosmetic(true);
   height -= pen.width() / 2;
   painter.setPen(pen);
   painter.drawLine(0, height, to, height);
   
   QRect bounds = this->rect();
   bounds.setWidth(to);
   bounds.setTop(bounds.y() + height);
   QLinearGradient gradient(bounds.topLeft(), bounds.bottomRight());
   gradient.setColorAt(0, Qt::transparent);
   gradient.setColorAt(1, shine);
   pen.setBrush(QBrush(gradient));
   painter.setPen(pen);
   painter.drawLine(0, height, to, height); // draw the line in reverse, so the "shine" is nearest to the "front"
}
void DKDelayedEnableButton::_draw_spinner(QPainter& painter, float progress, QRectF spinner) {
   QPen pen;
   pen.setColor(this->_spinner_options.background);
   pen.setWidthF(this->_spinner_options.thickness);
   pen.setCapStyle(Qt::RoundCap);
   pen.setCosmetic(true);
   
   painter.setRenderHint(QPainter::Antialiasing, true);
   {
      QPen border_pen = pen;
      border_pen.setWidthF(1.0F);
      border_pen.setColor(this->_spinner_options.border);
      //
      auto   thick = this->_spinner_options.thickness / 2.0F + 0.5F;
      QRectF rect  = spinner;
      rect += QMargins(thick, thick, thick, thick);
      painter.setPen(border_pen);
      painter.drawArc(rect, 0 * 16, 360 * 16);
      rect = spinner;
      rect -= QMargins(thick, thick, thick, thick);
      painter.setPen(border_pen);
      painter.drawArc(rect, 0 * 16, 360 * 16);
   }
   
   painter.setPen(pen);
   painter.drawArc(spinner, 0 * 16, 360 * 16);
   
   pen.setColor(this->_spinner_options.fill);
   if (this->_spinner_options.fillCoversBorder)
      pen.setWidthF(pen.width() + 2.0F);
   painter.setPen(pen);
   painter.drawArc(spinner, 90 * 16, -progress * (360 * 16)); // zero degrees is at 3 o'clock. angles are counterclockwise and can be negated for clockwise.
}
QString DKDelayedEnableButton::_effective_label() const {
   if (this->_style != CounterStyle::Number)
      return this->text();

   return tr("%1 (%2)").arg(this->text()).arg(this->secondsRemaining());
}