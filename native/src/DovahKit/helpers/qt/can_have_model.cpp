#include "can_have_model.h"
#include <QAbstractItemView>
#include <QComboBox>
#include <QSortFilterProxyModel>

namespace cobb::qt {
   extern bool can_have_model(const QWidget* root) {
      if (auto* casted = qobject_cast<QAbstractItemView*>(root))
         return true;
      if (auto* casted = qobject_cast<QComboBox*>(root))
         return true;
      return false;
   }
}