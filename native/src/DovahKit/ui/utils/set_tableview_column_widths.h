#pragma once
#include <type_traits>
#include <QAbstractItemModel>
#include <QFontMetrics>
#include <QHeaderView>
#include <QTableView>

namespace ui {
   //
   // The non-flex equivalent of `set_tableview_column_flex`. Useful for cases 
   // where the total widths of all columns will virtually always be wider than 
   // the width of the tableview, such that DKHeaderView's flex functionality 
   // would never actually do very much (and, if anything, might inadvertently 
   // shrink some columns into oblivion).
   //
   template<typename Callable> requires std::is_invocable_v<Callable, QHeaderView&, const QFontMetrics&>
   void set_tableview_column_widths(QTableView* widget, Callable&& callable) {
      auto* header = widget->horizontalHeader();
      //
      auto metrics = QFontMetrics(widget->font());
      header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignBaseline);
      header->setMinimumSectionSize(2);
      callable(*header, metrics);
      if (auto* model = widget->model()) {
         size_t count = model->columnCount();
         for (size_t i = 0; i < count; ++i)
            header->setSectionResizeMode(i, QHeaderView::Interactive);
      }
      header->setStretchLastSection(true);
   }
}