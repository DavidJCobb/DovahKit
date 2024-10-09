#include "./enable_inbound_drag_and_drop_insertions.h"

namespace ui {
   extern void enable_inbound_drag_and_drop_insertions(QAbstractItemView* view) {
      view->setAcceptDrops(true);
      view->setDragDropMode(QAbstractItemView::DragDropMode::DropOnly);
      view->setDragDropOverwriteMode(false);
      view->setDropIndicatorShown(true);
   }
}