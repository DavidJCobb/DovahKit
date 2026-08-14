#include "./set_textarea_height_in_lines.h"
#include <QPlainTextEdit>
#include <QTextEdit>

namespace {
   static int _desired_inner_height(QWidget& widget, size_t lines) {
      auto metrics = widget.fontMetrics();
      int  height  = metrics.height() * lines;
      if (lines > 1) {
         auto spacing = metrics.lineSpacing();
         height += spacing * (lines - 1);
      }
      return height;
   }
}

namespace ui {
   extern void set_textarea_height_in_lines(QPlainTextEdit& widget, size_t lines) {
      int margin = 0;
      if (auto* doc = widget.document())
         margin = doc->documentMargin();

      int height = (margin * 2) + _desired_inner_height(widget, lines);
      widget.resize(widget.width(), height);
   }
   extern void set_textarea_height_in_lines(QTextEdit& widget, size_t lines) {
      int margin = 0;
      if (auto* doc = widget.document())
         margin = doc->documentMargin();

      int height = (margin * 2) + _desired_inner_height(widget, lines);
      widget.resize(widget.width(), height);
   }
}