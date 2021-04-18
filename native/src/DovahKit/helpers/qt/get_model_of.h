#pragma once
#include <QAbstractItemModel>
#include <QWidget>

namespace cobb::qt {
   extern QAbstractItemModel* get_model_of(QWidget* root);

   // peeks below any QSortFilterProxyModel that may be present
   extern QAbstractItemModel* get_underlying_model_of(QWidget* root);
}

