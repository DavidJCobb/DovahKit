#pragma once
#include "DovahscriptCanvasWidgetLayerData.h"

class DovahscriptCanvasWidgetLayerDataText : public DovahscriptCanvasWidgetLayerData {
   Q_OBJECT;
   protected:
      struct {
         QSize size;
      } cache;
   public:
      DovahscriptCanvasWidgetLayerDataText();
      
      QString text;
      Qt::Alignment alignment = Qt::AlignLeft | Qt::AlignTop;
      QColor  color;
      QSizeF  constrain;
      QFont   font;
      bool    wordWrap = true;

      void update();

      virtual QRect rect() const noexcept;
   protected:
      virtual void paint(QPainter&, const QPoint pos, const QSize crop_to) noexcept;

      void paint_and_report(QPainter&, const QPoint& pos, QRectF& out_size) const noexcept;
};