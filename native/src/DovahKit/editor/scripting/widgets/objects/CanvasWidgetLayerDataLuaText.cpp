#include "CanvasWidgetLayerDataLuaText.h"
#include <QPainter>
#include <QTextLayout>

namespace {
   static constexpr int ABSURDLY_LARGE_SIZE = 9999;
}

CanvasWidgetLayerDataLuaText::CanvasWidgetLayerDataLuaText() {
   this->font.setPixelSize(12);
   this->font.setStyleHint(QFont::StyleHint::SansSerif);
   this->font.setFamilies({
      "Segoe UI",
      "Calibri",
      "Arial"
   });
}

void CanvasWidgetLayerDataLuaText::update() {
   this->cache.size = QSize();
   LuaScriptableCanvasWidgetLayerData::update();
}

void CanvasWidgetLayerDataLuaText::paint_and_report(QPainter& painter, const QPoint& pos, QRectF& out_size) const noexcept {
   if (this->text.isEmpty()) {
      out_size = QRectF(0, 0, 0, 0);
      return;
   }
   auto rect  = QRectF(0, 0, this->constrain.width(), this->constrain.height());
   int  flags = this->alignment;
   if (this->wordWrap) {
      flags |= Qt::TextWordWrap;
   }
   if (!(flags & Qt::AlignVertical_Mask)) { // default to top edge, not baseline
      flags |= Qt::AlignTop;
   }
   //
   painter.setFont(this->font);
   painter.setPen(this->color);
   if (rect.isValid()) {
      out_size = rect;
      if (this->wordWrap) {
         painter.drawText(rect, this->text, this->alignment);
      } else {
         auto prior = painter.clipBoundingRect();
         auto bound = rect.translated(pos);
         painter.setClipRect(bound);
         painter.drawText(pos, this->text);
         painter.setClipRect(prior);
      }
   } else {
      //
      // We are only constraining on one axis, or not constraining on any axis.
      // 
      // QPainter doesn't actually let you constrain the text only on one axis, so we have 
      // to work around that by setting the unconstrained axis to some absurdly long length. 
      // However, that in turn means that we need to be careful about text alignment on the 
      // unconstrained axis; for example, if width is unconstrained and we thus set the X 
      // axis to something like 9999, then text that's been flagged as right-aligned will 
      // draw out of bounds.
      //
      if (this->constrain.width() < 0) {
         flags &= ~Qt::AlignHorizontal_Mask; // strip horizontal flags
         flags |= Qt::AlignLeft | Qt::AlignAbsolute;
         rect.setWidth(ABSURDLY_LARGE_SIZE);
      }
      if (this->constrain.height() < 0) {
         flags &= ~Qt::AlignVertical_Mask; // strip vertical flags
         flags |= Qt::AlignTop;
         rect.setHeight(ABSURDLY_LARGE_SIZE);
      }
      auto prior = painter.clipBoundingRect();
      auto bound = rect.translated(pos);
      painter.setClipRect(bound);
      painter.drawText(rect, flags, this->text, &out_size);
      painter.setClipRect(prior);
   }
}
void CanvasWidgetLayerDataLuaText::paint(QPainter& painter, const QPoint pos, const QSize crop_to) noexcept {
   QRectF dummy;
   this->paint_and_report(painter, pos, dummy);
}
QRect CanvasWidgetLayerDataLuaText::rect() const noexcept {
   //
   // This probably seems pretty darned wasteful, huh? We have to draw the text twice, 
   // once to get the size and once to actually render it. Well, this is actually the 
   // best way. QPainter doesn't really give you a way to just measure text without 
   // drawing it to something, so we use a dummy 1x1px canvas.
   // 
   // We could measure the text on our own, but that would fail to account for a number 
   // of things, including:
   // 
   //  - QPainter rounds Y-offsets on lines of text to ensure that no line begins on a 
   //    subpixel.
   // 
   //  - Manual measurement will not account for italicized text taking up slightly 
   //    more width.
   //
   QRectF   out;
   QImage   dummy   = QImage(1, 1, QImage::Format::Format_Mono);
   QPainter painter = QPainter(&dummy);
   this->paint_and_report(painter, { 0, 0 }, out);
   return QRect(out.x(), out.y(), out.width(), out.height());
}