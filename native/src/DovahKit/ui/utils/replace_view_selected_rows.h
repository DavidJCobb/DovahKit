#pragma once
#include <QAbstractItemView>
#include <QItemSelectionModel>
#include <QModelIndex>

namespace ui {
   inline void replace_view_selected_rows(QItemSelectionModel& sm, const QModelIndex& qmi) {
      sm.select(qmi, QItemSelectionModel::SelectionFlag::ClearAndSelect | QItemSelectionModel::SelectionFlag::Rows);
   }
   inline void replace_view_selected_rows(QItemSelectionModel& sm, int row) {
      const auto* model = sm.model();
      if (!model)
         return;
      QModelIndex qmi;
      if (row >= 0) {
         qmi = model->index(row, 0, {});
         if (!qmi.isValid())
            return;
      }
      replace_view_selected_rows(sm, qmi);
   }

   inline void replace_view_selected_rows(QAbstractItemView& view, int row) {
      if (auto* sm = view.selectionModel())
         replace_view_selected_rows(*sm, row);
   }
   inline void replace_view_selected_rows(QAbstractItemView& view, const QModelIndex& qmi) {
      if (auto* sm = view.selectionModel())
         replace_view_selected_rows(*sm, qmi);
   }
}
