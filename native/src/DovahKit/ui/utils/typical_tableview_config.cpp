#include "./typical_tableview_config.h"
#include <QHeaderView>

namespace ui {
   extern void typical_tableview_config(QTableView* widget) {
      widget->setCornerButtonEnabled(false);
      widget->setDragDropOverwriteMode(false); // Qt defaults this to false for listviews and treeviews -- but true for tableviews, as a prank.
      widget->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
      widget->setHorizontalScrollMode(QAbstractItemView::ScrollMode::ScrollPerPixel);
      widget->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerItem);
      if (auto* vh = widget->verticalHeader()) {
         vh->setVisible(false);

         // Needed to get rid of unnecessary padding on rows, because Qt has very good and sane defaults.
         vh->setSectionResizeMode(QHeaderView::ResizeToContents);
      }
   }
}