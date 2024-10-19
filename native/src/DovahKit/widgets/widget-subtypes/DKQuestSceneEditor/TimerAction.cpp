#include "./TimerAction.h"
#include "./Style.h"
#include "../../DKQuestSceneEditor.h" // for QObject::tr

namespace DKQuestSceneEditor_impl {
   /*virtual*/ void TimerAction::paint(QPainter& painter, const Style& style) /*override*/ {
      painter.save();
      painter.translate(this->geometry.rect.topLeft());

      QString text;
      if (!this->name.isEmpty()) {
         text = DKQuestSceneEditor::tr("Action %1: %2 (%3 seconds)").arg(this->action_id).arg(this->name);
      } else {
         text = DKQuestSceneEditor::tr("Action %1 (%2 seconds)").arg(this->action_id);
      }
      text = text.arg(this->duration);

      QRect rect({ 0, 0 }, this->geometry.rect.size());
      painter.setBrush(QBrush(style.action.timer.background));
      painter.setPen(QPen(style.action.timer.text));
      painter.drawRect(rect);
      rect.adjust(0, style.action.header_padding, 0, 0);
      painter.drawText(rect, Qt::AlignTop | Qt::AlignHCenter, text);

      painter.restore();
   }
   /*virtual*/ void TimerAction::recalcSize(int width, const Style& style, const QFontMetrics& font_metrics) /*override*/ {
      this->geometry.rect.setWidth(width);
      this->geometry.rect.setHeight(font_metrics.lineSpacing() * 3 + style.action.header_padding * 2);
   }
}