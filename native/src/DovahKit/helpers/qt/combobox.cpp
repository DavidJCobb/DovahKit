#include "combobox.h"
#include <QSortFilterProxyModel>

namespace cobb::qt {
   extern int map_combobox_index_from_proxy(QComboBox* combobox, int index) {
      auto* proxy = qobject_cast<QSortFilterProxyModel*>(combobox->model());
      if (!proxy)
         return index;
      auto* model = proxy->sourceModel();
      auto  p_qmi = proxy->index(index, 0);
      auto  s_qmi = proxy->mapToSource(p_qmi);
      if (!s_qmi.isValid())
         return -1;
      return s_qmi.row();
   }
   extern int map_combobox_index_to_proxy(QComboBox* combobox, int index) {
      auto* proxy = qobject_cast<QSortFilterProxyModel*>(combobox->model());
      if (!proxy)
         return index;
      auto* model = proxy->sourceModel();
      auto  s_qmi = model->index(index, 0);
      if (!s_qmi.isValid())
         return -1;
      auto  p_qmi = proxy->mapFromSource(s_qmi);
      return p_qmi.row();
   }
}