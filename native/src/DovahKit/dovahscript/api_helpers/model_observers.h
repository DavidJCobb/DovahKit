#pragma once
#include <functional>
#include "../../ui/generic/ObservableStandardItemModel.h"
#include "../../lua.h"

namespace dovahscript::api_helpers {
   extern [[nodiscard]] QVariant get_model_items_data(ObservableStandardItemModelObserver* observer, int role);
   extern void set_model_items_data(ObservableStandardItemModelObserver* observer, int role, QVariant data);

   // Row/column numbers of -2 indicate "remove the whole span." Both == -2 indicate "clear the whole model."
   extern void remove_items_from_model(QWidget* widget, int row, int col, QModelIndex parent = QModelIndex());
}