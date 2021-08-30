#include "DovahscriptResourceStyledItemDelegate.h"
#include "DovahscriptResource.h"

namespace dovahscript {
   void DovahscriptResourceStyledItemDelegate::initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const {
      QStyledItemDelegate::initStyleOption(option, index);
      //
      auto data = index.data(Qt::DecorationRole);
      if (auto* resource = DovahscriptResourceHandle::extract_from_variant(data)) {
         switch (resource->resource_type()) {
            case resource_type::dds:
            case resource_type::raster:
               break;
            default:
               return;
         }
         const auto pm = resource->get_raster_widget_side();
         if (pm.isNull())
            return;
         option->icon = QIcon(QPixmap::fromImage(pm));
         {
            //option->decorationSize = pm.size() / pm.devicePixelRatio(); // displays icon at actual size
            auto size = pm.size() / pm.devicePixelRatio();
            auto max  = option->decorationSize;
            if (size.height() > max.height()) {
               option->decorationSize = size.boundedTo(max);
            } else if (size.height() < max.height()) {
               option->decorationSize = size.expandedTo(max);
            }
         }
         option->features |= QStyleOptionViewItem::HasDecoration;
      }
   }
}