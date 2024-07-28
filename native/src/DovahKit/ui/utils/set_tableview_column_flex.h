#pragma once
#include <type_traits>
#include <QAbstractItemModel>
#include <QFontMetrics>
#include <QTableView>
#include "widgets/DKHeaderView.h"

namespace ui {
   template<typename Callable> requires std::is_invocable_v<Callable, DKHeaderView&, const QFontMetrics&>
   void set_tableview_column_flex(QTableView* widget, Callable&& callable) {
      auto* header = new DKHeaderView(Qt::Orientation::Horizontal, widget);
      header->setFlexResizeEnabled(true);
      widget->setHorizontalHeader(header);
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
      header->setStretchLastSection(false);
   }
}