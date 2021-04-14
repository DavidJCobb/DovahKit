#pragma once
#include <QComboBox>

namespace cobb::qt {
   extern int map_combobox_index_from_proxy(QComboBox* combobox, int index);
   extern int map_combobox_index_to_proxy(QComboBox* combobox, int index);
}