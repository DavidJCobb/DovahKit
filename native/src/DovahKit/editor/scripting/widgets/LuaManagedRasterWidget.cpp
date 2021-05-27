#include "LuaManagedRasterWidget.h"
#include <QPaintEvent>
#include <QPainter>

LuaManagedRasterWidget::LuaManagedRasterWidget(QWidget* parent) : QWidget(parent) {
}

const QPixmap LuaManagedRasterWidget::_getPixmap() const noexcept {
   if (this->_resource)
      return this->_resource->get_raster_widget_side();
   return QPixmap();
}

void LuaManagedRasterWidget::setDesiredSize(const QSize& s) noexcept {
   if (this->_desiredSize == s)
      return;
   this->_desiredSize = s;
   this->updateGeometry();
}
void LuaManagedRasterWidget::setDesiredWidth(int s) noexcept {
   if (this->_desiredSize.width() == s)
      return;
   this->_desiredSize.setWidth(s);
   this->updateGeometry();
}
void LuaManagedRasterWidget::setDesiredHeight(int s) noexcept {
   if (this->_desiredSize.height() == s)
      return;
   this->_desiredSize.setHeight(s);
   this->updateGeometry();
}

void LuaManagedRasterWidget::setResource(const LMRH& input) {
   this->_resource = input;
   this->update();
}

int LuaManagedRasterWidget::heightForWidth(int w) const {
   auto pm = this->_getPixmap();
   if (pm.isNull())
      return QWidget::heightForWidth(w);
   double ratio = (double)pm.height() / pm.width();
   if (isnan(ratio))
      return 0;
   return w * ratio;
}
QSize LuaManagedRasterWidget::minimumSizeHint() const {
   QSize desired = this->_desiredSize;
   auto  pm      = this->_getPixmap();
   auto  w = desired.width();
   auto  h = desired.height();
   if (w < 0 && h < 0) {
      if (pm.isNull())
         return QSize(0, 0);
      return pm.size();
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
QSize LuaManagedRasterWidget::sizeHint() const {
   auto pm = this->_getPixmap();
   if (pm.isNull())
      return QSize(-1, -1);
   return pm.size();
}

void LuaManagedRasterWidget::paintEvent(QPaintEvent* event) {
   auto pm = this->_getPixmap();
   if (pm.isNull())
      return;
   QPainter painter(this);
   painter.drawPixmap(0, 0, width(), height(), pm);
}