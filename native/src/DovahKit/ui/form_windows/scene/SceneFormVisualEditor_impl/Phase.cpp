#include "./Phase.h"
#include "./Style.h"
#include "./StyleOption.h"
#include "../SceneFormVisualEditor.h" // for QObject::tr

#include "editor/helpers/stringify_conditions.h"

namespace SceneFormVisualEditor_impl {
   void Phase::paint(QPainter& painter, const Style& style, const StyleOption& option, int index, int height) {
      painter.setBrush(QBrush(style.phase.background));
      painter.setPen(QPen(style.phase.text));

      painter.save();
      painter.translate(this->geometry.rect.topLeft());

      QRect rect;
      rect.setWidth(this->geometry.rect.width());
      rect.setHeight(height);
      painter.drawRect(rect);
      if (option.selected) {
         //
         // Draw selection border.
         //
         if (option.active) {
            painter.setBrush(QBrush(style.selection.background));
         } else {
            painter.setBrush(QBrush(style.selection.inactive.background));
         }
         painter.setPen(QPen(Qt::PenStyle::NoPen));
         rect.adjust(-2, -2, 2, 2);
         painter.drawRect(rect);
         //
         // Reset.
         //
         painter.setBrush(QBrush(style.selection.background));
         painter.setPen(QPen(style.selection.text));
      }

      {  // Header
         QString text;
         if (this->name.isEmpty()) {
            text = SceneFormVisualEditor::tr("Action %1: %2").arg(index).arg(this->name);
         } else {
            text = SceneFormVisualEditor::tr("Action %1").arg(index);
         }
         const QRect& rect = this->geometry.rel.header;
         if (option.selected && option.active) {
            painter.setBrush(QBrush(style.selection.background));
            painter.setPen(QPen(style.selection.text));
         }
         painter.drawRect(rect);
         painter.drawText(rect, Qt::AlignTop | Qt::AlignHCenter, text);
         if (option.selected && option.active) {
            painter.setBrush(QBrush(style.phase.background));
            painter.setPen(QPen(style.phase.text));
         }
      }
      if (auto src = this->cached.conditions.start; !src.isEmpty()) {
         QString text = SceneFormVisualEditor::tr("Start conditions:\n") + src;

         const QRect& rect = this->geometry.rel.conditions.start;
         painter.drawRect(rect);
         painter.drawText(rect, Qt::AlignTop | Qt::AlignLeft, text);
      }
      if (auto src = this->cached.conditions.completion; !src.isEmpty()) {
         QString text = SceneFormVisualEditor::tr("Completion conditions:\n") + src;

         const QRect& rect = this->geometry.rel.conditions.completion;
         painter.drawRect(rect);
         painter.drawText(rect, Qt::AlignTop | Qt::AlignLeft, text);
      }

      painter.restore();
   }
   void Phase::recacheConditionStrings(const ui::types::conditions::context& context) {
      this->cached.conditions = {};
      {
         auto& src = this->conditions.start;
         auto& dst = this->cached.conditions.start;
         if (!src.empty()) {
            for (size_t i = 0; i < src.size() - 1; ++i) {
               auto& cnd = src.back();
               dst += editor_helpers::stringify_condition(src[i], context);
               dst += editor_helpers::stringify_condition_boolean_operator(src[i]);
            }
            dst += editor_helpers::stringify_condition(src.back(), context);
         }
      }
      {
         auto& src = this->conditions.completion;
         auto& dst = this->cached.conditions.completion;
         if (!src.empty()) {
            for (size_t i = 0; i < src.size() - 1; ++i) {
               auto& cnd = src.back();
               dst += editor_helpers::stringify_condition(src[i], context);
               dst += editor_helpers::stringify_condition_boolean_operator(src[i]);
            }
            dst += editor_helpers::stringify_condition(src.back(), context);
         }
      }
   }
   void Phase::recalcSize(int width, const Style& style, const QFontMetrics& font_metrics) {
      this->geometry.rect.setWidth(width);

      int height    = 0;
      int box_width = width - style.phase.header_box.margin * 2;
      {
         auto& box = this->geometry.rel.header;
         box.setX(style.phase.header_box.margin);
         box.setY(style.phase.header_box.margin);
         box.setWidth(box_width);
         box.setHeight(font_metrics.lineSpacing() * 2);
         height = box.bottom() + style.phase.header_box.margin;
      }
      int condition_box_height = font_metrics.lineSpacing() * 4;
      if (this->cached.conditions.start.isEmpty()) {
         this->geometry.rel.conditions.start = {};
      } else {
         int y = this->geometry.rel.header.bottom() + style.phase.header_box.margin * 2;

         auto& box  = this->geometry.rel.conditions.start;
         auto& prev = this->geometry.rel.header;
         box.setX(style.phase.header_box.margin);
         box.setY(y);
         box.setWidth(box_width);
         box.setHeight(condition_box_height);
         height = box.bottom() + style.phase.header_box.margin;
      }
      if (this->cached.conditions.completion.isEmpty()) {
         this->geometry.rel.conditions.completion = {};
      } else {
         int y = style.phase.header_box.margin * 2;
         if (this->cached.conditions.start.isEmpty()) {
            y += this->geometry.rel.header.bottom();
         } else {
            y += this->geometry.rel.conditions.start.bottom();
         }

         auto& box  = this->geometry.rel.conditions.start;
         auto& prev = this->geometry.rel.header;
         box.setX(style.phase.header_box.margin);
         box.setY(y);
         box.setWidth(box_width);
         box.setHeight(condition_box_height);
         height = box.bottom() + style.phase.header_box.margin;
      }

      this->geometry.rect.setHeight(height);
   }
}