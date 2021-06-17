#include "CanvasWidgetLayerDataLuaText.h"
#include <QPainter>
#include <QTextLayout>

namespace {
   static constexpr int ABSURDLY_LARGE_SIZE = 9999;
   static constexpr int MAX_LINE_COUNT      = 5000;
}

namespace {
   // TODO: Test if this works. If we can measure how QPainter would lay out and 
   // draw text by drawing it to a 1x1px dummy image, then we can use that to 
   // guarantee a consistent size on our rect calls (though arguably we should 
   // cache that).
   QSizeF test_measure_text(CanvasWidgetLayerDataLuaText& data) {
      int flags = data.alignment;
      if (data.wordWrap)
         flags |= Qt::TextWordWrap;
      //
      auto constrain = QRectF({ 0, 0 }, data.constrain);
      if (!constrain.isValid()) {
         if (constrain.width() <= 0) {
            constrain.setWidth(ABSURDLY_LARGE_SIZE);
         }
         if (constrain.height() <= 0) {
            constrain.setHeight(ABSURDLY_LARGE_SIZE);
         }
      }
      //
      QImage   dummy   = QImage(1, 1, QImage::Format::Format_Mono);
      QPainter painter = QPainter(&dummy);
      QRectF   bounding;
      painter.setFont(data.font);
      painter.setPen(data.color);
      painter.drawText(constrain, flags, data.text, &bounding);
      //
      auto br = bounding.bottomRight();
      return { br.x(), br.y() };
   }
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

void CanvasWidgetLayerDataLuaText::paint(QPainter& painter, const QPoint& pos) noexcept {
   if (this->text.isEmpty())
      return;
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
      if (this->constrain.width() > 0) {
         flags &= ~Qt::AlignVertical_Mask; // strip vertical flags
         flags |=  Qt::AlignTop;
         rect.setHeight(ABSURDLY_LARGE_SIZE);
      } else if (this->constrain.height() > 0) {
         flags &= ~Qt::AlignHorizontal_Mask; // strip horizontal flags
         flags |=  Qt::AlignLeft | Qt::AlignAbsolute;
         rect.setWidth(ABSURDLY_LARGE_SIZE);
      }
      if (rect.isValid()) {
         auto prior = painter.clipBoundingRect();
         auto bound = rect.translated(pos);
         painter.setClipRect(bound);
         painter.drawText(rect, flags, this->text);
         painter.setClipRect(prior);
      } else {
         //
         // For the drawText function that takes a point, the Y-position is the baseline, not 
         // the top edge.
         //
         QFontMetrics metrics(this->font);
         auto ascent = metrics.ascent();
         //
         painter.drawText(pos + QPoint(0, ascent), this->text);
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
      //
      // TODO: This won't be perfectly consistent with QPainter because QPainter actually 
      // goes out of its way to ensure that each line begins on an integer coordinate, to 
      // avoid sub-pixel blurs. We should test to see if (test_measure_text), above, will 
      // work properly.
      //
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