#include "./Action.h"
#include "./Style.h"

namespace SceneFormVisualEditor_impl {
   void Action::drawCell(QPainter& painter, const Style& style, QRect rect, QString text, int align_flags) const {
      QPen border_pen;
      border_pen.setCosmetic(true);
      border_pen.setJoinStyle(Qt::PenJoinStyle::MiterJoin);
      border_pen.setWidth(1);
      //
      // Draw border outer:
      //
      border_pen.setColor(style.action.cell.border.outer_dark);
      painter.setPen(border_pen);
      painter.drawLine(rect.topLeft(), rect.topRight());
      painter.drawLine(rect.topLeft(), rect.bottomLeft());
      border_pen.setColor(style.action.cell.border.outer_light);
      painter.setPen(border_pen);
      painter.drawLine(rect.bottomRight(), rect.bottomLeft());
      painter.drawLine(rect.bottomRight(), rect.topRight());
      rect.adjust(1, 1, -1, -1);
      //
      // Draw border inner:
      //
      border_pen.setColor(style.action.cell.border.inner_dark);
      painter.setPen(border_pen);
      painter.drawLine(rect.topLeft(), rect.topRight());
      painter.drawLine(rect.topLeft(), rect.bottomLeft());
      border_pen.setColor(style.action.cell.border.inner_light);
      painter.setPen(border_pen);
      painter.drawLine(rect.bottomRight(), rect.bottomLeft());
      painter.drawLine(rect.bottomRight(), rect.topRight());
      rect.adjust(1, 1, -1, -1);
      //
      painter.setBrush(QBrush(style.action.cell.background));
      painter.setPen(Qt::NoPen);
      painter.drawRect(rect);
      //
      // Draw text:
      //
      int padding = style.action.cell.padding;
      rect.adjust(padding, padding, -padding, -padding); // row padding
      painter.setPen(QPen(style.action.cell.text));
      painter.drawText(rect, align_flags, text);
   }
}