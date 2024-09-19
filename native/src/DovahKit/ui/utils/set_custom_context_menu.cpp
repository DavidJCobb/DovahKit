#include "./set_custom_context_menu.h"

namespace ui {
   extern void set_custom_context_menu(QWidget& widget, QMenu& menu) {
      widget.setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
      QObject::connect(&widget, &QWidget::customContextMenuRequested, &menu, [&menu, &widget](const QPoint& pos) {
         menu.exec(widget.mapToGlobal(pos));
      });
   }
}