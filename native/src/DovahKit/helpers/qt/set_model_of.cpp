#include "set_model_of.h"
#include <QAbstractItemView>
#include <QComboBox>

namespace cobb::qt {
   extern void set_model_of(QWidget* widget, QAbstractItemModel* model) {
      if (auto* casted = qobject_cast<QAbstractItemView*>(widget)) {
         casted->setModel(model);
         return;
      }
      if (auto* casted = qobject_cast<QComboBox*>(widget)) {
         casted->setModel(model);
         return;
      }
      #if _DEBUG
         __debugbreak(); // This widget can't have a model!
      #endif
   }
}

