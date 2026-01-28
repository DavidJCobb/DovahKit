#pragma once
#include <QPainter>

namespace ui::utils {
   namespace paint {
      struct ArrowStyle {
         struct {
            float  angle  = 22.5F;
            QBrush brush;
            float  length = 10.0F;
            QPen   pen;
         } head;
         struct {
            QPen pen;
         } stem;
      };
   }
   
   void paint_arrow(
      QPainter&,
      const QPointF& src_point,
      const QPointF& dst_point,
      const float    dst_margin,
      const paint::ArrowStyle&
   );
}