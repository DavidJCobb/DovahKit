#include "layout.h"
#include <QLayout>

namespace cobb::qt {
   extern void remove_layout(QWidget* widget) {
      auto* layout = widget->layout();
      if (!layout)
         return;
      QLayoutItem* item = nullptr;
      while ((item = layout->takeAt(0)) != nullptr) {
         delete item;
      }
   }
}