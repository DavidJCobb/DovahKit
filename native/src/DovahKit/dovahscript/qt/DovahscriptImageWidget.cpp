#include "DovahscriptImageWidget.h"
#include <QPaintEvent>
#include <QPainter>

DovahscriptImageWidget::DovahscriptImageWidget(QWidget* parent) : QWidget(parent) {
   this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

const QImage DovahscriptImageWidget::_getContent() const noexcept {
   if (this->_resource)
      return this->_resource->get_raster_widget_side();
   return QImage();
}

void DovahscriptImageWidget::setDesiredSize(const QSize& s) noexcept {
   if (this->_desiredSize == s)
      return;
   this->_desiredSize = s;
   this->updateGeometry();
}
void DovahscriptImageWidget::setDesiredWidth(int s) noexcept {
   if (this->_desiredSize.width() == s)
      return;
   this->_desiredSize.setWidth(s);
   this->updateGeometry();
}
void DovahscriptImageWidget::setDesiredHeight(int s) noexcept {
   if (this->_desiredSize.height() == s)
      return;
   this->_desiredSize.setHeight(s);
   this->updateGeometry();
}

void DovahscriptImageWidget::setResource(const DSRH& input) {
   if (this->_resource) {
      QObject::disconnect(this->_resource, nullptr, this, nullptr);
   }
   this->_resource = input;
   this->updateGeometry();
   this->update();
   QObject::connect(this->_resource, &DSR::resynchronized, this, [this]() {
      this->updateGeometry();
      this->update();
   });
}

bool DovahscriptImageWidget::hasHeightForWidth() const {
   return false;
   return !this->_getContent().isNull();
}
int DovahscriptImageWidget::heightForWidth(int w) const {
   auto pm = this->_getContent();
   if (pm.isNull())
      return QWidget::heightForWidth(w);
   double ratio = (double)pm.height() / pm.width();
   if (isnan(ratio))
      return 0;
   return w * ratio;
}
QSize DovahscriptImageWidget::minimumSizeHint() const {
   QSize desired = this->_desiredSize;
   auto  pm      = this->_getContent();
   auto  w = desired.width();
   auto  h = desired.height();
   if (w < 0 && h < 0) {
      if (pm.isNull())
         return QSize(0, 0);
      return pm.size() / pm.devicePixelRatio();
   }
   if (w < 0) {
      if (pm.isNull())
         return QSize(0, h);
      double ratio = (double)pm.width() / pm.height();
      desired.setWidth(isnan(ratio) ? 0 : ratio * h);
      return desired;
   }
   if (h < 0) {
      if (pm.isNull())
         return QSize(w, 0);
      desired.setHeight(this->heightForWidth(w));
      return desired;
   }
   return QSize(w, h);
}
QSize DovahscriptImageWidget::sizeHint() const {
   auto pm = this->_getContent();
   if (pm.isNull())
      return QSize(-1, -1);
   return pm.size() / pm.devicePixelRatio();
}

void DovahscriptImageWidget::paintEvent(QPaintEvent* event) {
   auto pm = this->_getContent();
   if (pm.isNull())
      return;
   QSize space = this->size();
   QSize size  = (pm.size() / pm.devicePixelRatio()).scaled(space.width(), space.height(), Qt::AspectRatioMode::KeepAspectRatio);
   int x = (space.width() - size.width()) / 2;
   int y = (space.height() - size.height()) / 2;
   //
   QPainter painter(this);
   painter.drawImage(x, y, pm, size.width(), size.height());
}