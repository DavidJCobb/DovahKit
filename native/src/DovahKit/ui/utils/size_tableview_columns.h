#pragma once
#include <type_traits>
#include <QAbstractItemModel>
#include <QFontMetrics>
#include <QStyle>
#include <QStyleOption>
#include <QTableView>
#include "helpers/type_traits/is_std_array.h"
#include "widgets/DKHeaderView.h"

namespace ui {
   struct tableview_column_spec {
      public:
         static constexpr const size_t auto_width = 0;

      public:
         size_t grow   = 1;
         size_t shrink = 1;
         size_t width  = auto_width;
   };

   namespace impl {
      template<typename T>
      concept is_tableview_column_spec_array = requires {
         requires cobb::is_std_array<T>;
         requires std::is_same_v<typename T::value_type, tableview_column_spec>;
      };
   }

   template<auto ColumnSpecs> requires impl::is_tableview_column_spec_array<decltype(ColumnSpecs)>
   void size_tableview_columns(QTableView* widget) {
      auto* header = new DKHeaderView(Qt::Orientation::Horizontal, widget);
      auto* style  = header->style();
      header->setFlexResizeEnabled(true);
      widget->setHorizontalHeader(header);
      header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignBaseline);
      header->setMinimumSectionSize(2);
      header->setStretchLastSection(false);

      auto metrics = QFontMetrics(widget->font());
      int  padding = 0;
      {
         QStyleOptionHeader option;
         option.initFrom(header);
         padding = style->pixelMetric(QStyle::PM_HeaderMargin, &option, header) * 2;

         // golly gee it'd be nice if qt's accessors for sizing metrics actually, uh, worked
         padding += 4; // blind guess
      }

      auto* model = widget->model();
      if (!model)
         return;

      size_t count = std::min((size_t)model->columnCount(), ColumnSpecs.size());
      for (size_t i = 0; i < count; ++i) {
         const auto& spec = ColumnSpecs[i];
         {
            int basis = spec.width;
            if (basis == tableview_column_spec::auto_width) {
               auto text = model->headerData(i, Qt::Orientation::Horizontal, Qt::DisplayRole).toString();
               basis = metrics.size(0, text).width() + padding;
            }
            header->setColumnFlex(i, spec.grow, spec.shrink, basis);
         }
         header->setSectionResizeMode(i, QHeaderView::Interactive);
      }
   }
}