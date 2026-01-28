#include "./paint_single_line_text.h"
#include <QPainterPath>

namespace ui::utils {
   extern void paint_single_line_text(QPainter& painter, QPointF at, Qt::Alignment align, QString text) {
      const auto font_metrics = painter.fontMetrics();
      if (align & (Qt::AlignmentFlag::AlignHCenter | Qt::AlignmentFlag::AlignRight)) {
         QRect bounds = font_metrics.boundingRect(text);
         if (align & Qt::AlignmentFlag::AlignHCenter) {
            at.rx() -= bounds.width() / 2;
         } else if (align & Qt::AlignmentFlag::AlignRight) {
            at.rx() -= bounds.width();
         }
      }
      if (align & Qt::AlignmentFlag::AlignTop) {
         at.ry() += font_metrics.ascent();
      } else if (align & Qt::AlignmentFlag::AlignBottom) {
         at.ry() -= font_metrics.descent();
      } else if (align & Qt::AlignmentFlag::AlignVCenter) {
         at.ry() += font_metrics.ascent();
         const auto line_height = font_metrics.ascent() + font_metrics.descent();
         at.ry() -= line_height / 2;
      }

      QPainterPath path;
      path.addText(
         at.x(),
         at.y(),
         painter.font(),
         text
      );
      painter.strokePath(path, painter.pen());
      painter.fillPath(path, painter.brush());
   }
}
