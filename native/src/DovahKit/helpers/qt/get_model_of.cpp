#include "get_model_of.h"
#include <QAbstractItemView>
#include <QComboBox>
#include <QSortFilterProxyModel>

namespace cobb::qt {
   extern QAbstractItemModel* get_model_of(QWidget* root) {
      if (auto* casted = qobject_cast<QAbstractItemView*>(root))
         return casted->model();
      if (auto* casted = qobject_cast<QComboBox*>(root))
         return casted->model();
      return nullptr;
   }
   extern QAbstractItemModel* get_underlying_model_of(QWidget* root) {
      auto* model = get_model_of(root);
      if (model) {
         while (auto* proxy = qobject_cast<QSortFilterProxyModel*>(model))
            model = proxy->sourceModel();
      }
      return model;
   }
}

