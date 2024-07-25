#include "./item_indices_to_data.h"

namespace ui {
   extern void item_indices_to_data(QComboBox* widget, Qt::ItemDataRole role) {
      size_t size = widget->count();
      for (size_t i = 0; i < size; ++i)
         widget->setItemData(i, (int)i, role);
   }
}