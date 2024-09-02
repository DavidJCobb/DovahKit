#pragma once
#include <QItemSelectionModel>

namespace ui {
   extern QModelIndex get_selected_row_qmi(QItemSelectionModel* sel_model) {
      auto rows = sel_model->selectedRows();
      if (rows.isEmpty())
         return {};
      return rows[0];
   }
}
