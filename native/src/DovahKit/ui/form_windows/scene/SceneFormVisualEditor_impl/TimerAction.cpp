#include "./TimerAction.h"
#include "./Style.h"
#include "./StyleOption.h"
#include "../SceneFormVisualEditor.h" // for QObject::tr

namespace SceneFormVisualEditor_impl {
   /*virtual*/ void TimerAction::paint(QPainter& painter, const Style& style, const StyleOption& option) /*override*/ {
      painter.save();
      painter.translate(this->geometry.rect.topLeft());
      if (option.selected) {
         //
         // Draw selection border.
         //
         auto rect = this->geometry.rect;
         rect.moveTo(0, 0);
         rect.adjust(-2, -2, 2, 2);
         if (option.active) {
            painter.setBrush(QBrush(style.selection.background));
         } else {
            painter.setBrush(QBrush(style.selection.inactive.background));
         }
         painter.setPen(QPen(Qt::PenStyle::NoPen));
         painter.drawRect(rect);
      }

      QString text;
      if (!this->base_data.name.isEmpty()) {
         text = SceneFormVisualEditor::tr("Action %1: %2 (%3 seconds)").arg(this->base_data.action_id).arg(this->base_data.name);
      } else {
         text = SceneFormVisualEditor::tr("Action %1 (%2 seconds)").arg(this->base_data.action_id);
      }
      text = text.arg(this->data.duration);

      QRect rect({ 0, 0 }, this->geometry.rect.size());
      if (option.selected) {
         painter.setBrush(QBrush(style.selection.background));
      } else {
         painter.setBrush(QBrush(style.action.timer.background));
      }
      painter.setPen(QPen(style.action.timer.text));
      painter.drawRect(rect);
      rect.adjust(0, style.action.header_padding, 0, 0);
      if (option.selected) {
         painter.setPen(QPen(style.selection.text));
      }
      painter.drawText(rect, Qt::AlignTop | Qt::AlignHCenter, text);

      painter.restore();
   }
   /*virtual*/ void TimerAction::recalcSize(int width, const Style& style, const QFontMetrics& font_metrics) /*override*/ {
      this->geometry.rect.setWidth(width);
      this->geometry.rect.setHeight(font_metrics.lineSpacing() * 3 + style.action.header_padding * 2);
   }
}