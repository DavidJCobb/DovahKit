#pragma once
#include <QComboBox>

namespace ui {
   //
   // Loop over all items currently in the combobox, and set their data for a given role 
   // to their index within the combobox. Useful alongside ui::bind, if you've already 
   // added items to a combobox via Qt Designer and want their indices to map to their 
   // underlying values.
   //
   extern void item_indices_to_data(QComboBox*, Qt::ItemDataRole role = Qt::ItemDataRole::UserRole);
}