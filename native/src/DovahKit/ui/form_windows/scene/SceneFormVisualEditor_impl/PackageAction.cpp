#include "./PackageAction.h"
#include "dovah/form_stub.h"
#include "editor/helpers/form_identifiers_to_string.h"
#include "./Style.h"
#include "./StyleOption.h"
#include "../SceneFormVisualEditor.h" // for QObject::tr

namespace SceneFormVisualEditor_impl {
   /*virtual*/ void PackageAction::paint(QPainter& painter, const Style& style, const StyleOption& option) /*override*/ {
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
         painter.setBrush(QBrush(style.action.package_header.background));
      }
      painter.setPen(QPen(style.action.package_header.text));
      painter.drawRect(QRect{ QPoint{ 0, 0 }, this->geometry.rect.size()});
      painter.drawRect(this->geometry.rel.header);
      if (option.selected) {
         painter.setPen(QPen(style.selection.text));
      }
      painter.drawText(this->geometry.rel.header, Qt::AlignTop | Qt::AlignHCenter, text);

      size_t size = this->packages.size();
      for (size_t i = 0; i < size; ++i) {
         int y = this->geometry.rel.body.y() + this->body_geometry.row_height * i;

         QRect row = this->geometry.rel.body;
         row.setY(y);
         row.setHeight(this->body_geometry.row_height);

         QString form_id   = "00000000";
         QString editor_id;
         if (i < this->cached.package_editor_ids.size()) {
            editor_id = this->cached.package_editor_ids[i];
         }
         if (auto* stub = this->packages[i]) {
            form_id = editor_helpers::form_id_to_string(stub->formID);
            if (editor_id.isEmpty())
               editor_id = QString::fromStdString(stub->editorID);
         }

         {
            QRect cell = row;
            cell.setWidth(this->body_geometry.form_id_width);
            this->drawCell(painter, style, cell, form_id, Qt::AlignTop | Qt::AlignLeft);
         }
         if (row.width() > this->body_geometry.form_id_width) {
            QRect cell = row;
            cell.setX(cell.x() + this->body_geometry.form_id_width);
            this->drawCell(painter, style, cell, editor_id, Qt::AlignTop | Qt::AlignHCenter);
         }
      }

      painter.restore();
   }
   /*virtual*/ void PackageAction::recalcSize(int width, const Style& style, const QFontMetrics& font_metrics) /*override*/ {
      this->geometry.rect.setWidth(width);

      int height = font_metrics.lineSpacing(); // header
      height += style.action.header_padding * 2;
      height += 1; // border
      const int header_height = height;
      this->geometry.rel.header = QRect(QPoint{ 0, 0 }, QSize{ width, header_height });

      this->body_geometry.row_height = font_metrics.lineSpacing();
      this->body_geometry.row_height += style.action.cell.padding * 2;
      this->body_geometry.row_height += cell_border_width * 2;
      height += this->body_geometry.row_height * this->packages.size();

      this->geometry.rel.body = QRect(QPoint{ 0, header_height }, QSize{ width, height - header_height });

      this->body_geometry.form_id_width = font_metrics.horizontalAdvance("00000000");
      this->body_geometry.form_id_width += style.action.cell.padding * 2;
      this->body_geometry.form_id_width += cell_border_width * 2;

      this->geometry.rect.setHeight(height);
   }
}