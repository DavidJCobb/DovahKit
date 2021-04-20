#pragma once
#include "../../../ui/util/lua_item_model.h"

namespace editor_script::helpers {
   extern QVariant get_model_items_data(ObservableStandardItemModelObserver* observer, int role);
   extern void set_model_items_data(ObservableStandardItemModelObserver* observer, int role, QVariant data);
}