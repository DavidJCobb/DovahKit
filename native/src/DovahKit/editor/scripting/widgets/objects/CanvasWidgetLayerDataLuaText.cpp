#include "CanvasWidgetLayerDataLuaText.h"
#include <QPainter>
#include <QTextLayout>

namespace {
   static constexpr int MAX_LINE_COUNT = 5000;
}

void CanvasWidgetLayerDataLuaText::update() {
   this->cache.size = QSize();
   LuaScriptableCanvasWidgetLayerData::update();
}

void CanvasWidgetLayerDataLuaText::paint(QPainter& painter, const QPoint& pos) noexcept {
   if (this->text.isEmpty())
      return;
   QRect rect = QRect(0, 0, this->constrain.width(), this->constrain.height());
   //
   painter.setFont(this->font);
   painter.setPen(this->color);
   if (rect.isValid()) {
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
      if (this->constrain.width() > 0) {
         rect.setHeight(9999);
      } else if (this->constrain.height() > 0) {
         rect.setWidth(9999);
      }
      if (rect.isValid()) {
         auto prior = painter.clipBoundingRect();
         auto bound = rect.translated(pos);
         painter.setClipRect(bound);
         painter.drawText(pos, this->text);
         painter.setClipRect(prior);
      } else {
         painter.drawText(pos, this->text);
      }
   }
}
QRect CanvasWidgetLayerDataLuaText::rect() const noexcept {
   if (this->text.isEmpty())
      return QRect();
   auto w = this->constrain.width();
   auto h = this->constrain.height();
   if (w > 0 && h > 0) {
      return QRect(0, 0, w, h);
   }
   QFontMetrics metrics(this->font);
   if (w > 0) {
      if (!this->wordWrap) {
         int fw = metrics.horizontalAdvance(this->text);
         int fh = metrics.height();
         if (fw > w)
            fw = w;
         return QRect(0, 0, fw, fh);
      }
      QTextLayout layout(this->text, this->font);
      layout.beginLayout();
      for (int i = 0; i < MAX_LINE_COUNT; ++i) {
         QTextLine line = layout.createLine();
         if (!line.isValid())
            break;
         line.setLineWidth(w);
      }
      layout.endLayout();
      //
      int fw = w;
      int fh = layout.lineCount() * metrics.lineSpacing();
      return QRect(0, 0, fw, fh);
   } else if (h > 0) {
      int fw = metrics.horizontalAdvance(this->text);
      int fh = metrics.height();
      if (fh > h)
         fh = h;
      return QRect(0, 0, fw, fh);
   } else {
      int fw = metrics.horizontalAdvance(this->text);
      int fh = metrics.height();
      return QRect(0, 0, fw, fh);
   }
}