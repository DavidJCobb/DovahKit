#pragma once
#include <QStyledItemDelegate>

namespace dovahscript {
   class DovahscriptResourceStyledItemDelegate : public QStyledItemDelegate {
      public:
         using QStyledItemDelegate::QStyledItemDelegate;

         virtual void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override;
   };
}