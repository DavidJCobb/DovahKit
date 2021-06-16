#pragma once
#include "LuaScriptableCanvasWidgetLayerData.h"

class CanvasWidgetLayerDataLuaText : public LuaScriptableCanvasWidgetLayerData {
   Q_OBJECT;
   protected:
      struct {
         QSize size;
      } cache;
   public:
      QString text;
      Qt::Alignment alignment;
      QColor  color;
      QSizeF  constrain;
      QFont   font;
      bool    wordWrap = true;

      void update();

      virtual QRect rect() const noexcept;
   protected:
      virtual void paint(QPainter&, const QPoint& pos) noexcept;
};