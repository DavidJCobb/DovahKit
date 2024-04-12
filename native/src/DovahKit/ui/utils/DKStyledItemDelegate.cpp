#include "./DKStyledItemDelegate.h"
#include <QApplication>
#include <QMouseEvent>

/*virtual*/ void DKStyledItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const /*override*/ {
   bool is_aligned_checkbox = false;
   {
      if (index.isValid()) {
         auto _variant_is_empty = [](const QVariant& v) { return !v.isValid() || v.isNull(); };
         //
         if (
            !_variant_is_empty(index.data(Qt::TextAlignmentRole)) &&
            !_variant_is_empty(index.data(Qt::CheckStateRole)) &&
            _variant_is_empty(index.data(Qt::DecorationRole)) &&
            _variant_is_empty(index.data(Qt::DisplayRole))
         ) {
            is_aligned_checkbox = true;
         }
      }
   }
   if (!is_aligned_checkbox)
      return QStyledItemDelegate::paint(painter, option, index);
   //
   // If we made it here, then this item consists of a single checkbox only, with a text alignment set.
   //
   auto option_local = option;
   this->initStyleOption(&option_local, index);

   auto* widget = option_local.widget;
   auto* style  = widget ? widget->style() : QApplication::style();
   //
   // Draw the background first.
   //
   option_local.features &= ~(QStyleOptionViewItem::HasDisplay | QStyleOptionViewItem::HasDecoration | QStyleOptionViewItem::HasCheckIndicator);
   style->drawControl(QStyle::CE_ItemViewItem, &option_local, painter, widget);
   //
   // Draw the checkbox.
   //
   option_local.rect   = _get_checkbox_rect(option_local);
   option_local.state &= ~(QStyle::State_On | QStyle::State_Off | QStyle::State_NoChange | QStyle::State_HasFocus);
   switch (option_local.checkState) {
      case Qt::Checked:
         option_local.state |= QStyle::State_On;
         break;
      case Qt::PartiallyChecked:
         option_local.state |= QStyle::State_NoChange;
         break;
      case Qt::Unchecked:
         option_local.state |= QStyle::State_Off;
         break;
   }
   style->drawPrimitive(QStyle::PE_IndicatorItemViewItemCheck, &option_local, painter, widget);
}

/*virtual*/ bool DKStyledItemDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) /*override*/ {
   auto flags = index.flags();
   if (!(flags & (Qt::ItemFlag::ItemIsUserCheckable | Qt::ItemFlag::ItemIsEnabled)))
      return false;

   switch (event->type()) {
      case QEvent::Type::MouseButtonRelease:
         {
            auto* casted = (QMouseEvent*)event;
            if (!_get_checkbox_rect(option).contains(casted->pos()))
               return false;
         }
         break;
      case QEvent::Type::KeyPress:
         {
            auto* casted = (QKeyEvent*)event;
            if (casted->key() != Qt::Key_Space)
               return false;
         }
         break;
      default:
         return false;
   }

   auto prior = index.data(Qt::CheckStateRole).value<Qt::CheckState>();
   auto after = prior == Qt::Checked ? Qt::Unchecked : Qt::Checked;
   return model->setData(index, after, Qt::CheckStateRole);
}

QRect DKStyledItemDelegate::_get_checkbox_rect(const QStyleOptionViewItem& option) const {
   auto* widget        = option.widget;
   auto* style         = widget ? widget->style() : QApplication::style();
   auto  checkbox_size = style->subElementRect(QStyle::SE_CheckBoxIndicator, &option, widget).size();
   return QStyle::alignedRect(option.direction, option.displayAlignment, checkbox_size, option.rect);
}