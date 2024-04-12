#pragma once
#include <QStyledItemDelegate>

//
// Features:
// 
//  - If a cell has a checkbox, and no text or icon, then Qt::TextAlignmentRole applies to the checkbox.
//
class DKStyledItemDelegate : public QStyledItemDelegate {
   public:
      using QStyledItemDelegate::QStyledItemDelegate;

      virtual bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;
      virtual void paint(QPainter* painter, const QStyleOptionViewItem& o, const QModelIndex& index) const override;

   protected:
      QRect _get_checkbox_rect(const QStyleOptionViewItem& option) const;
};