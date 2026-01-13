#pragma once
#include <QSortFilterProxyModel>
#include "dovah/form_types.h"

class _FormSignatureComboboxProxy : public QSortFilterProxyModel {
   Q_OBJECT
   //
   // Special proxy that always sorts the "unfiltered" option ahead of all the others.
   //
   public:
      _FormSignatureComboboxProxy(QObject* parent = nullptr) : QSortFilterProxyModel(parent) {}
      bool lessThan(const QModelIndex& left, const QModelIndex& right) const override {
         auto source    = this->sourceModel();
         auto sort_role = this->sortRole();
         //
         auto a = (dovah::form_type)source->data(left,  Qt::UserRole).toInt();
         auto b = (dovah::form_type)source->data(right, Qt::UserRole).toInt();
         if (a != b) {
            if (a == dovah::form_type::none)
               return true;
            if (b == dovah::form_type::none)
               return false;
         }
         //
         QVariant leftData  = source->data(left,  sort_role);
         QVariant rightData = source->data(right, sort_role);
         return QString::localeAwareCompare(leftData.toString(), rightData.toString()) < 0;
      }
};