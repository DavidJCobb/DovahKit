#include "./paint_multiline_text.h"
#include <QPainterPath>

namespace ui::utils {
   extern void paint_multiline_text(QPainter& painter, const QPointF& at, Qt::Alignment align, QString text) {
      QPainterPath path;

      const auto font_metrics = painter.fontMetrics();
      const auto line_height  = font_metrics.ascent() + font_metrics.descent();
      const auto line_count   = text.count('\n') + 1;

      float y = at.y();
      if (align & Qt::AlignmentFlag::AlignTop) {
         y += font_metrics.ascent();
      } else if (align & Qt::AlignmentFlag::AlignBottom) {
         y -= font_metrics.descent();
         y -= (line_height + font_metrics.leading()) * (line_count - 1);
      } else if (align & Qt::AlignmentFlag::AlignVCenter) {
         y += font_metrics.ascent();

         int text_block_height = (line_height * line_count) + (font_metrics.leading() * (line_count - 1));
         y -= text_block_height / 2;
      }

      const auto distance_per_line = line_height + font_metrics.leading();
      int from  = 0;
      int until = text.indexOf('\n', from);
      do {
         if (until == from) {
            do {
               y    += distance_per_line;
               from  = until + 1;
               until = text.indexOf('\n', from);
               if (until < 0)
                  break;
            } while (until == from);
            if (until < 0)
               break;
         }
         QString fragment;
         if (until < 0)
            fragment = text.mid(from);
         else
            fragment = text.mid(from, until - from);

         float x = at.x();
         if (align & (Qt::AlignmentFlag::AlignHCenter | Qt::AlignmentFlag::AlignRight)) {
            QRect bounds = font_metrics.boundingRect(fragment);
            if (align & Qt::AlignmentFlag::AlignHCenter) {
               x -= bounds.width() / 2;
            } else if (align & Qt::AlignmentFlag::AlignRight) {
               x -= bounds.width();
            }
         }
         path.addText(x, y, painter.font(), fragment);
         y += distance_per_line;

         if (until < 0)
            break;
         from  = until + 1;
         until = text.indexOf('\n', from);
      } while (true);

      painter.strokePath(path, painter.pen());
      painter.fillPath(path, painter.brush());
   }
}