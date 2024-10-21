#include "./DialogueAction.h"
#include "./Style.h"
#include "./StyleOption.h"
#include "../SceneFormVisualEditor.h" // for QObject::tr

namespace SceneFormVisualEditor_impl {
   /*virtual*/ void DialogueAction::paint(QPainter& painter, const Style& style, const StyleOption& option) /*override*/ {
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
      if (!this->name.isEmpty()) {
         text = SceneFormVisualEditor::tr("Action %1: %2").arg(this->action_id).arg(this->name);
      } else {
         text = SceneFormVisualEditor::tr("Action %1").arg(this->action_id);
      }

      if (option.selected) {
         painter.setBrush(QBrush(style.selection.background));
      } else {
         painter.setBrush(QBrush(style.action.dialogue_header.background));
      }
      painter.setPen(QPen(style.action.dialogue_header.text));
      painter.drawRect(QRect{ QPoint{ 0, 0 }, this->geometry.rect.size()});
      painter.drawRect(this->geometry.rel.header);
      if (option.selected) {
         painter.setPen(QPen(style.selection.text));
      }
      painter.drawText(this->geometry.rel.header, Qt::AlignTop | Qt::AlignHCenter, text);

      int cell_flags = Qt::AlignTop | Qt::AlignLeft;
      if (style.show_all_text)
         cell_flags |= Qt::TextWordWrap;

      size_t size = this->cached.infos.size();
      int    y    = this->geometry.rel.body.y();
      for (size_t i = 0; i < size; ++i) {
         auto text   = this->cached.infos[i];
         auto height = this->body_geometry.infos[i];

         QRect row = this->geometry.rel.body;
         row.setY(y);
         row.setHeight(height);
         this->drawCell(painter, style, row, text, cell_flags);

         y += height;
      }

      painter.restore();
   }
   /*virtual*/ void DialogueAction::recalcSize(int width, const Style& style, const QFontMetrics& font_metrics) /*override*/ {
      this->geometry.rect.setWidth(width);

      int height = font_metrics.lineSpacing(); // header
      height += style.action.header_padding * 2;
      height += 1; // border
      const int header_height = height;
      this->geometry.rel.header = QRect(QPoint{ 0, 0 }, QSize{ width, header_height });

      size_t count = this->cached.infos.size();
      auto&  dst   = this->body_geometry.infos;
      dst.clear();
      dst.resize(count);
      for (size_t i = 0; i < count; ++i) {
         if (style.show_all_text) {
            auto text_rect = font_metrics.boundingRect(
               0,
               0,
               width,
               std::numeric_limits<int>::max(),
               Qt::TextSingleLine | Qt::TextWordWrap,
               this->cached.infos[i]
            );
            dst[i] = text_rect.bottom();
         } else {
            dst[i] = font_metrics.lineSpacing();
         }
         dst[i] += style.action.cell.padding * 2;
         dst[i] += cell_border_width * 2;

         height += dst[i];
      }

      this->geometry.rel.body = QRect(QPoint{ 0, header_height }, QSize{ width, height - header_height });
      this->geometry.rect.setHeight(height);
   }
}