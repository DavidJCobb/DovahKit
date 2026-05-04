#include "DKUnreadCountBadge.h"
#include <QBoxLayout>
#include <QPaintEvent>
#include <QPainter>

DKUnreadCountBadge::DKUnreadCountBadge(QWidget* parent) : QWidget(parent) {
   auto* l = this->_text = new QLabel(this);
   l->setAlignment(Qt::AlignCenter);
   {
      auto* layout = new QHBoxLayout(this);
      this->setLayout(layout);
      layout->setContentsMargins({ 4, 0, 4, 0 });
      layout->addWidget(l);
      layout->setSizeConstraint(QLayout::SetFixedSize);
   }
   {
      auto f = this->font();
      f.setBold(true);
      this->setFont(f);
   }
   this->setBackgroundColor(QColor(255, 0, 0));
   this->setTextColor(QColor(255, 255, 255));
   this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
   l->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
   //
   auto fm = QFontMetrics(l->font());
   this->setMinimumHeight(fm.height() * 1.2);
   this->setMinimumWidth(fm.height() * 1.2);
   //
   this->_format = "%1";
   this->setCount(0);
}

void DKUnreadCountBadge::_updateFont() {
   auto* widget = this->_text;
   widget->setFont(this->font());
}
void DKUnreadCountBadge::_updateText() {
   auto* widget = this->_text;
   widget->setText(this->_format.arg(this->_count));
}

QColor DKUnreadCountBadge::backgroundColor() const noexcept {
   return this->palette().color(QPalette::ColorRole::Base);
}
void DKUnreadCountBadge::setBackgroundColor(const QColor& c) {
   auto palette = this->palette();
   palette.setColor(QPalette::ColorRole::Base, c);
   this->setPalette(palette);
   this->update();
}

QColor DKUnreadCountBadge::textColor() const noexcept {
   auto* widget  = this->_text;
   auto  palette = widget->palette();
   return palette.color(QPalette::ColorRole::WindowText);
}
void DKUnreadCountBadge::setTextColor(const QColor& c) {
   auto* widget  = this->_text;
   auto  palette = widget->palette();
   palette.setColor(QPalette::ColorRole::WindowText, c);
   palette.setColor(QPalette::ColorRole::Text, c);
   widget->setPalette(palette);
}

void DKUnreadCountBadge::setCount(int v) {
   this->_count = v;
   this->setVisible(this->_showIfZero || (v != 0));
   this->_updateText();
}
void DKUnreadCountBadge::setFormat(const QString& t) {
   this->_format = t;
   this->_updateText();
}
void DKUnreadCountBadge::setShowIfZero(bool b) {
   if (this->_showIfZero == b)
      return;
   this->_showIfZero = b;
   this->setVisible(b || (this->_count != 0));
}

QSize DKUnreadCountBadge::minimumSizeHint() const noexcept {
   return this->_text->sizeHint();
}
QSize DKUnreadCountBadge::sizeHint() const noexcept {
   return this->_text->sizeHint();
}
void DKUnreadCountBadge::changeEvent(QEvent* event) {
   switch (event->type()) {
      case QEvent::FontChange:
         this->_updateFont();
         break;
   }
}
void DKUnreadCountBadge::paintEvent(QPaintEvent* event) {
   auto  rect  = this->rect();
   qreal round = rect.height() / 2;
   //
   QPainter painter(this);
   painter.setPen(QPen(QBrush(), 0));
   painter.setBrush(this->backgroundColor());
   painter.drawRoundedRect(rect, round, round);
}