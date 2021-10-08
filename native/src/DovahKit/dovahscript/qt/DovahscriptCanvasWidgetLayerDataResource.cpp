#include "DovahscriptCanvasWidgetLayerDataResource.h"
#include <QPainter>

namespace {
   using DSRH = dovahscript::DovahscriptResourceUIHandle;
}

void DovahscriptCanvasWidgetLayerDataResource::paint(QPainter& painter, const QPoint pos, const QSize crop_to) noexcept {
   auto* resource = this->_handle.bare();
   if (!resource)
      return;
   switch (resource->resource_type()) {
      using t = dovahscript::resource_type;
      case t::dds:
      case t::raster:
         break;
      default:
         return;
   }
   auto content = resource->get_raster_widget_side();
   if (crop_to.isValid())
      painter.drawImage(0, 0, content, pos.x(), pos.y(), crop_to.width(), crop_to.height());
   else
      painter.drawImage(pos, content);
}
QRect DovahscriptCanvasWidgetLayerDataResource::rect() const noexcept {
   auto* resource = this->_handle.bare();
   if (!resource)
      return QRect();
   switch (resource->resource_type()) {
      using t = dovahscript::resource_type;
      case t::dds:
      case t::raster:
         return resource->get_raster_widget_side().rect();
   }
   return QRect();
}

DSRH DovahscriptCanvasWidgetLayerDataResource::resource() {
   return this->_handle;
}
void DovahscriptCanvasWidgetLayerDataResource::setResource(DSRH r) {
   this->_handle = r;
   this->update();
}